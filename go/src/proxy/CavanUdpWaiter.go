package proxy

import (
	"time"
)

type CavanUdpWaiter struct {
	Next     *CavanUdpWaiter
	Link     *CavanUdpLink
	OpCode   CavanUdpOpCode
	PackChan chan *CavanUdpPack
}

func NewCavanUdpWaiter(link *CavanUdpLink, op CavanUdpOpCode) *CavanUdpWaiter {
	waiter := CavanUdpWaiter{Link: link, OpCode: op}
	waiter.PackChan = make(chan *CavanUdpPack, 2)
	return &waiter
}

func (waiter *CavanUdpWaiter) WaitReady(delay time.Duration) *CavanUdpPack {
	if waiter.Link.Closed && waiter.OpCode != CavanUdpOpClose {
		return nil
	}

	select {
	case pack := <-waiter.PackChan:
		return pack

	case <-time.After(delay):
		return nil
	}
}

func (waiter *CavanUdpWaiter) SetReady(pack *CavanUdpPack) {
	waiter.PackChan <- pack
}
