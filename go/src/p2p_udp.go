package main

import (
	"fmt"
	"net"
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
	proxy.NewCavanUdpTurnServer(0)
	time.Sleep(time.Hour)
}
