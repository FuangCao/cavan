package main

import (
	"flag"
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
	os.Args = os.Args[1:]

	flag_help := flag.Bool("help", false, "Show help message")
	flag_port := flag.Int("port", 8867, "Tcp listener port")

	if command == "server" {
		flag.Parse()

		if *flag_help {
			flag.Usage()
			return
		}

		proxy.NewCavanUdpTurnServer(*flag_port)
	} else if command == "client" {
		flag_server := flag.String("server", "free.qydev.com:4044", "Role change server url")
		flag_udp_server := flag.String("udp-server", "127.0.0.1:8867", "Udp server url")
		flag_name := flag.String("name", "", "Role change client name")
		flag_proxy := flag.String("proxy", "127.0.0.1:8021", "Proxy url")

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
