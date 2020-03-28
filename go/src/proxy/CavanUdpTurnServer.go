package proxy

import (
	"fmt"
	"net"
	"strconv"
	"strings"
	"time"

	"../common"
)

type CavanUdpTurnServer struct {
	Sock     *CavanUdpSock
	Listener *net.TCPListener
}

func NewCavanUdpTurnServer(port int) *CavanUdpTurnServer {
	fmt.Println("port =", port)

	sock := NewCavanUdpSock()
	if sock == nil {
		return nil
	}

	addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%d", port))
	if err != nil {
		return nil
	}

	fmt.Println("addr =", addr)

	listener, err := net.ListenTCP("tcp", addr)
	if err != nil {
		return nil
	}

	server := CavanUdpTurnServer{Sock: sock, Listener: listener}
	go server.TcpMainLoop()

	return &server
}

func (server *CavanUdpTurnServer) TcpMainLoop() {
	for true {
		conn, err := server.Listener.Accept()
		if err != nil {
			fmt.Println(err)
			time.Sleep(time.Second)
		} else {
			go server.TcpDaemonLoop(conn)
		}
	}
}

func (server *CavanUdpTurnServer) TcpDaemonLoop(conn net.Conn) error {
	defer conn.Close()

	command, err := common.CavanConnReadPackStr(conn, time.Second*20)
	if err != nil {
		fmt.Println(err)
		return err
	}

	fmt.Println("command =", command)

	if strings.HasPrefix(string(command), "link") {
		command, err = common.CavanConnReadPackStr(conn, time.Second*20)
		if err != nil {
			fmt.Println(err)
			return err
		}

		fmt.Println("command =", command)
	}

	args := strings.Split(command, " ")
	fmt.Println(args)

	if len(args) != 2 {
		return nil
	}

	addr, err := net.ResolveUDPAddr("udp", args[0])
	if err != nil {
		fmt.Println(err)
		return err
	}

	fmt.Println(addr)

	port, err := strconv.Atoi(args[1])
	if err != nil {
		fmt.Println(err)
		return err
	}

	wan := server.Sock.GetWanAddr()
	if wan == nil {
		fmt.Println("Failed to GetWanAddr")
		return nil
	}

	fmt.Println("wan = ", wan)

	udp := server.Sock.NewLink(addr)
	if udp == nil {
		return nil
	}

	udp.RemotePort = uint16(port)

	fmt.Println(udp)

	response := fmt.Sprintf("%s %d", wan.String(), udp.LocalPort)
	common.CavanConnWritePack(conn, []byte(response))

	if !udp.SendPingSync() {
		return nil
	}

	fmt.Println("success")

	return nil
}
