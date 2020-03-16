package proxy

import (
	"time"
)

type CavanUdpWaiter struct {
	Next     *CavanUdpWaiter
	PackType CavanUdpPackType
	PackChan chan *CavanUdpPack
}

func NewCavanUdpWaiter(id CavanUdpPackType) *CavanUdpWaiter {
	waiter := CavanUdpWaiter{PackType: id}
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
