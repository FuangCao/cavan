package main

import (
	"fmt"
	"net"
	"os"
	"time"

	"./common"
	"./proxy"
	"./stun"
)

func GetWanAddr(conn *net.UDPConn) *net.UDPAddr {
	addr, err := net.ResolveUDPAddr("udp", "stun1.l.google.com:19302")
	if err != nil {
		fmt.Println(err)
		return nil
	}

	for true {
		conn.SetReadDeadline(time.Now().Add(time.Second))

		ostream := common.NewByteStream(20)
		ostream.WriteValueBe16(1)
		conn.WriteToUDP(ostream.Bytes, addr)

		response := make([]byte, 2048)
		len, addr, err := conn.ReadFromUDP(response)
		if err != nil {
			fmt.Println(err)
			continue
		}

		fmt.Println(addr)

		pack := stun.StunPackDecode(response, len)

		return pack.GetWanAddr()
	}

	return nil
}

type CavanUdpCommand struct {
	proxy.CavanUdpCmdNode
}

func main() {
	if len(os.Args) < 2 {
		return
	}

	command := os.Args[1]

	if command == "server" {
		fmt.Println("NewCavanUdpTurnServer")
		proxy.NewCavanUdpTurnServer(8867)
	} else if command == "client" {
		client := proxy.NewCavanUdpTurnClient(1234, "free.qydev.com:4044", "127.0.0.1:8867", "127.0.0.1:8021", "")
		fmt.Println("NewUdpLink = ", client.NewUdpLink())
	} else {
		fmt.Printf("Invalid command: %s\n", command)
	}

	for true {
		fmt.Println("=============================")
		time.Sleep(time.Minute)
	}
}
