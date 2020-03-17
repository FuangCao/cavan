package proxy

import (
	"time"
)

type CavanUdpWaiter struct {
	Next     *CavanUdpWaiter
	OpCode   CavanUdpOpCode
	PackChan chan *CavanUdpPack
}

func NewCavanUdpWaiter(op CavanUdpOpCode) *CavanUdpWaiter {
	waiter := CavanUdpWaiter{OpCode: op}
	waiter.PackChan = make(chan *CavanUdpPack, 2)
	return &waiter
}

func (waiter *CavanUdpWaiter) WaitReady(delay time.Duration) *CavanUdpPack {
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
