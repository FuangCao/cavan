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
	sock := NewCavanUdpSock()
	if sock == nil {
		return nil
	}

	addr, err := net.ResolveTCPAddr("tcp", ":8867")
	if err != nil {
		return nil
	}

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
		fmt.Println(conn)

		if err != nil {
			fmt.Println(err)
			time.Sleep(time.Second)
		} else {
			go server.TcpDaemonLoop(conn)
		}
	}
}

func (server *CavanUdpTurnServer) TcpDaemonLoop(conn net.Conn) {
	defer conn.Close()

	for true {
		command, err := common.CavanConnReadPack(conn)
		if err != nil {
			fmt.Println(err)
			break
		}

		args := strings.Split(string(command), " ")
		if len(args) != 2 {
			fmt.Println(args)
			break
		}

		addr, err := net.ResolveUDPAddr("udp", args[0])
		if err != nil {
			fmt.Println(err)
			break
		}

		port, err := strconv.Atoi(args[1])
		if err != nil {
			fmt.Println(err)
			break
		}

		wan := server.Sock.GetWanAddr()
		if wan == nil {
			break
		}

		udp := server.Sock.NewLink(addr)
		if udp == nil {
			break
		}

		udp.RemotePort = uint16(port)

		fmt.Println(udp)

		response := fmt.Sprintf("%s %d", wan.String(), udp.LocalPort)
		common.CavanConnWritePack(conn, []byte(response))
	}
}
