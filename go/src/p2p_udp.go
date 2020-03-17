package main

import (
	"fmt"
	"os"
	"time"

	"./proxy"
)

func main() {
	if len(os.Args) < 2 {
		return
	}

	command := os.Args[1]

	if command == "server" {
		fmt.Println("NewCavanUdpTurnServer")
		proxy.NewCavanUdpTurnServer(8867)
	} else if command == "client" {
		fmt.Println("NewCavanUdpTurnClient")
		proxy.NewCavanUdpTurnClient(1234, "free.qydev.com:4044", "127.0.0.1:8867", "127.0.0.1:8021", "")
	} else {
		fmt.Printf("Invalid command: %s\n", command)
	}

	for true {
		fmt.Println("=============================")
		time.Sleep(time.Minute)
	}
}
