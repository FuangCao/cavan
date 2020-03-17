package proxy

import (
	"fmt"
	"net"
	"strconv"
	"strings"
	"sync"
	"time"

	"../common"
)

type CavanUdpTurnClient struct {
	sync.Mutex
	ClientName string
	ClientUrl  string
	ServerUrl  string
	ProxyUrl   string
	Sock       *CavanUdpSock
	UdpCtrl    *CavanUdpLink
	Listener   *net.TCPListener
}

func NewCavanUdpTurnClient(port int, server string, client string, proxy string, name string) *CavanUdpTurnClient {
	fmt.Println("port =", port)
	fmt.Println("server =", server)
	fmt.Println("client =", client)
	fmt.Println("proxy =", proxy)
	fmt.Println("name =", name)

	addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%d", port))
	if err != nil {
		fmt.Println(err)
		return nil
	}

	fmt.Println("addr =", addr)

	listener, err := net.ListenTCP("tcp", addr)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	sock := NewCavanUdpSock()
	if sock == nil {
		return nil
	}

	udp := CavanUdpTurnClient{Sock: sock, Listener: listener, ServerUrl: server, ClientUrl: client, ProxyUrl: proxy, ClientName: name}
	go udp.TcpMainLoop()

	return &udp
}

func (client *CavanUdpTurnClient) TcpMainLoop() {
	for true {
		conn, err := client.Listener.Accept()
		if err != nil {
			fmt.Println(err)
			time.Sleep(time.Second)
			continue
		}

		go client.TcpDaemonLoop(conn)
	}
}

func (client *CavanUdpTurnClient) TcpDaemonLoop(conn net.Conn) {
	defer conn.Close()

	udp := client.NewUdpLink()
	if udp == nil {
		fmt.Println("Failed to NewUdpLink")
		return
	}

	udp.ProxyConn = conn

	udp.ProxyLoop(conn)
}

func (client *CavanUdpTurnClient) BuildProxyCommand() string {
	if len(client.ClientName) == 0 {
		return fmt.Sprintf("link %s", client.ClientUrl)
	}

	return fmt.Sprintf("link %s %s", client.ClientName, client.ClientUrl)
}

func (client *CavanUdpTurnClient) GetUdpCtrl() *CavanUdpLink {
	client.Lock()
	defer client.Unlock()

	ctrl := client.UdpCtrl
	if ctrl != nil && ctrl.SendPingSync() {
		return ctrl
	}

	wan := client.Sock.GetWanAddr()
	if wan == nil {
		fmt.Println("Failed to GetWanAddr")
		return nil
	}

	fmt.Println("wan = ", wan)

	ctrl = client.Sock.NewLink(nil)
	if ctrl == nil {
		fmt.Println("Failed to NewLink")
		return nil
	}

	addr, err := net.ResolveTCPAddr("tcp", client.ServerUrl)
	if err != nil {
		fmt.Println(err)
		ctrl.Close()
		return nil
	}

	fmt.Println("tcp = ", addr)

	conn, err := net.DialTCP("tcp", nil, addr)
	if err != nil {
		fmt.Println(err)
		ctrl.Close()
		return nil
	}

	defer conn.Close()

	command := client.BuildProxyCommand()
	if err := common.CavanConnWritePack(conn, []byte(command)); err != nil {
		fmt.Println(err)
		ctrl.Close()
		return nil
	}

	command = fmt.Sprintf("%s %d", wan.String(), ctrl.LocalPort)
	if err := common.CavanConnWritePack(conn, []byte(command)); err != nil {
		fmt.Println(err)
		ctrl.Close()
		return nil
	}

	response, err := common.CavanConnReadPack(conn, time.Second*20)
	if err != nil {
		fmt.Println(err)
		ctrl.Close()
		return nil
	}

	args := strings.Split(string(response), " ")
	if len(args) != 2 {
		fmt.Println(args)
		ctrl.Close()
		return nil
	}

	fmt.Println(args)

	if len(args) < 2 {
		ctrl.Close()
		return nil
	}

	if err := ctrl.SetRemoteAddr(args[0]); err != nil {
		fmt.Println(err)
		ctrl.Close()
		return nil
	}

	port, err := strconv.Atoi(args[1])
	if err != nil {
		fmt.Println(err)
		ctrl.Close()
		return nil
	}

	ctrl.RemotePort = uint16(port)
	client.UdpCtrl = ctrl

	return ctrl
}

func (client *CavanUdpTurnClient) NewUdpLink() *CavanUdpLink {
	ctrl := client.GetUdpCtrl()
	if ctrl == nil {
		fmt.Println("Failed to GetUdpCtrl")
		return nil
	}

	link := client.Sock.NewLink(ctrl.Addr)
	if link == nil {
		fmt.Println("Failed to NewLink")
		return nil
	}

	url := []byte(client.ProxyUrl)

	builder := NewCavanUdpCmdBuilder(CavanUdpOpConn, len(url)+2)
	builder.AppendValue16(link.LocalPort)
	builder.AppendBytes(url)

	if response := builder.Build(ctrl).SendWaitResponse(time.Millisecond * 500); response != nil {
		bytes := response.Bytes
		if len(bytes) < 8 {
			link.Close()
			return nil
		}

		link.RemotePort = uint16(common.DecodeValue16(bytes, 6))
	} else {
		link.Close()
		return nil
	}

	fmt.Println("success")

	return link
}
