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
	Links      int
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

		// fmt.Println("TcpMainLoop:", conn)

		go client.TcpDaemonLoop(conn)
	}
}

func (client *CavanUdpTurnClient) OnConnected(conn net.Conn) {
	client.Lock()
	client.Links++
	fmt.Println("OnConnected:", conn.RemoteAddr(), "links =", client.Links)
	client.Unlock()
}

func (client *CavanUdpTurnClient) OnDisconnected(conn net.Conn) {
	conn.Close()

	client.Lock()
	client.Links--
	fmt.Println("OnDisconnected:", conn.RemoteAddr(), "links =", client.Links)
	client.Unlock()
}

func (client *CavanUdpTurnClient) TcpDaemonLoop(conn net.Conn) {
	defer func() {
		conn.Close()
		client.OnDisconnected(conn)
	}()

	client.OnConnected(conn)

	udp := client.NewUdpLink()
	if udp == nil {
		fmt.Println("Failed to NewUdpLink")
		return
	} else {
		udp.ProxyConn = conn
		udp.ProxyLoop(conn)
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
		ctrl.Close(false)
		return nil
	}

	fmt.Println("tcp = ", addr)

	conn, err := net.DialTCP("tcp", nil, addr)
	if err != nil {
		fmt.Println(err)
		ctrl.Close(false)
		return nil
	}

	defer conn.Close()

	command := client.BuildProxyCommand()
	if err := common.CavanConnWritePack(conn, []byte(command)); err != nil {
		fmt.Println(err)
		ctrl.Close(false)
		return nil
	}

	command = fmt.Sprintf("%s %d", wan.String(), ctrl.LocalPort)
	if err := common.CavanConnWritePack(conn, []byte(command)); err != nil {
		fmt.Println(err)
		ctrl.Close(false)
		return nil
	}

	response, err := common.CavanConnReadPack(conn, time.Second*20)
	if err != nil {
		fmt.Println(err)
		ctrl.Close(false)
		return nil
	}

	args := strings.Split(string(response), " ")
	if len(args) != 2 {
		fmt.Println(args)
		ctrl.Close(false)
		return nil
	}

	fmt.Println(args)

	if len(args) < 2 {
		ctrl.Close(false)
		return nil
	}

	if err := ctrl.SetRemoteAddr(args[0]); err != nil {
		fmt.Println(err)
		ctrl.Close(false)
		return nil
	}

	port, err := strconv.Atoi(args[1])
	if err != nil {
		fmt.Println(err)
		ctrl.Close(false)
		return nil
	}

	ctrl.RemotePort = uint16(port)
	client.UdpCtrl = ctrl

	return ctrl
}

func (client *CavanUdpTurnClient) NewUdpLink() *CavanUdpLink {
	ctrl := client.GetUdpCtrl()
	// fmt.Println("ctrl = ", ctrl)
	if ctrl == nil {
		fmt.Println("Failed to GetUdpCtrl")
		return nil
	}

	link := client.Sock.NewLink(ctrl.Addr)
	// fmt.Println("link = ", link)
	if link == nil {
		fmt.Println("Failed to NewLink")
		return nil
	}

	fmt.Println("url =", client.ProxyUrl)
	url := []byte(client.ProxyUrl)

	builder := NewCavanUdpCmdBuilder(CavanUdpOpConn, len(url)+2)
	builder.AppendValue16(link.LocalPort)
	builder.AppendBytes(url)

	if response := builder.Build(ctrl).SendWaitResponse(time.Millisecond * 500); response != nil {
		bytes := response.Bytes
		if len(bytes) < 8 {
			link.Close(false)
			return nil
		}

		link.RemotePort = uint16(common.DecodeValue16(bytes, 6))
	} else {
		link.Close(false)
		return nil
	}

	fmt.Println("success")

	return link
}
