package main

import (
	"flag"
	"fmt"
	"os"
	"time"

	"./proxy"
)

const (
	ROLE_CHANGE_URL string = "free.qydev.com:4044"
	UDP_SERVER_PORT int    = 8867
)

func main() {
	if len(os.Args) < 2 {
		args := make([]string, 2)
		args[0] = os.Args[0]
		args[1] = "client"
		os.Args = args
	}

	command := os.Args[1]
	os.Args = os.Args[1:]

	flag_help := flag.Bool("help", false, "Show help message")

	if command == "server" {
		flag_port := flag.Int("port", UDP_SERVER_PORT, "Tcp listener port")

		flag.Parse()

		if *flag_help {
			flag.Usage()
			return
		}

		proxy.NewCavanUdpTurnServer(*flag_port)
	} else if command == "client" {
		flag_port := flag.Int("port", 9090, "Tcp listener port")
		flag_server := flag.String("server", ROLE_CHANGE_URL, "Role change server url")
		flag_udp_server := flag.String("udp-server", fmt.Sprintf("127.0.0.1:%d", UDP_SERVER_PORT), "Udp server url")
		flag_name := flag.String("name", "", "Role change client name")
		flag_proxy := flag.String("proxy", "127.0.0.1:9090", "Proxy url")

		flag.Parse()

		if *flag_help {
			flag.Usage()
			return
		}

		proxy.NewCavanUdpTurnClient(*flag_port, *flag_server, *flag_udp_server, *flag_proxy, *flag_name)
	} else {
		fmt.Printf("Invalid command: %s\n", command)
	}

	for true {
		fmt.Println(time.Now())
		time.Sleep(time.Minute)
	}
}
