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
	addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%d", port))
	if err != nil {
		fmt.Println(err)
		return nil
	}

	fmt.Println(addr)

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
		return
	}

	defer udp.Close()

	udp.ProxyConn = conn

	bytes := make([]byte, 1024)

	for true {
		conn.SetReadDeadline(time.Now().Add(time.Minute * 5))
		length, err := conn.Read(bytes)
		if err != nil {
			fmt.Println(err)
			break
		}

		udp.SendDataAsync(bytes[0:length])
	}
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
	if ctrl != nil && ctrl.SendPing() {
		return ctrl
	}

	wan := client.Sock.GetWanAddr()
	if wan == nil {
		return nil
	}

	fmt.Println("wan = ", wan)

	ctrl = client.Sock.NewLink(nil)
	if ctrl == nil {
		return nil
	}

	addr, err := net.ResolveTCPAddr("tcp", client.ServerUrl)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	fmt.Println("tcp = ", addr)

	conn, err := net.DialTCP("tcp", nil, addr)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	fmt.Println("conn = ", conn)

	command := client.BuildProxyCommand()
	if err := common.CavanConnWritePack(conn, []byte(command)); err != nil {
		fmt.Println(err)
		return nil
	}

	command = fmt.Sprintf("%s %d", wan.String(), ctrl.LocalPort)
	if err := common.CavanConnWritePack(conn, []byte(command)); err != nil {
		fmt.Println(err)
		return nil
	}

	response, err := common.CavanConnReadPack(conn, time.Second*20)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	args := strings.Split(string(response), " ")
	if len(args) != 2 {
		fmt.Println(args)
		return nil
	}

	fmt.Println(args)

	if len(args) < 2 {
		return nil
	}

	if err := ctrl.SetRemoteAddr(args[0]); err != nil {
		fmt.Println(err)
		return nil
	}

	if port, err := strconv.Atoi(args[1]); err != nil {
		fmt.Println(err)
		return nil
	} else {
		ctrl.RemotePort = uint16(port)
	}

	client.UdpCtrl = ctrl

	return ctrl
}

func (client *CavanUdpTurnClient) NewUdpLink() *CavanUdpLink {
	ctrl := client.GetUdpCtrl()
	if ctrl == nil {
		return nil
	}

	link := client.Sock.NewLink(ctrl.Addr)
	if link == nil {
		return nil
	}

	url := []byte(client.ProxyUrl)

	builder := NewCavanUdpCmdBuilder(CavanUdpOpConn, len(url)+2)
	builder.AppendValue16(link.LocalPort)
	builder.AppendBytes(url)

	if response := builder.Build(ctrl).SendWaitResponse(time.Millisecond * 500); response != nil {
		fmt.Println(response.Bytes)
		bytes := response.Bytes
		if len(bytes) < 8 {
			return nil
		}

		link.RemotePort = uint16(common.DecodeValue16(bytes, 6))
	} else {
		return nil
	}

	fmt.Println("success")

	return link
}
