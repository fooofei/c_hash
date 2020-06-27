package xhash

import (
	"errors"
	"sync/atomic"
)

const (
	DataMask = 0x3FF_FFFF_FFFF_FFFF
)

var (
	ErrNotFound = errors.New("not found in table")
	ErrNoRoom = errors.New("not have enough room")
)

/**
 * |---------------|----------|----------|----------|---------------|
 *
 *   0-3read count 4dowrite    5is_tail
 *  最多支持 2^4 -1=15 个线程同时读取
 */

func getReadCount(k uint64) uint64 {
	return (k >> 60) & 0xF
}

func getWrite(k uint64) bool {
	if (k>>59)&0x1 == 0x01 {
		return true
	}
	return false
}

func getTail(k uint64) bool {
	if (k>>58)&0x1 == 0x01 {
		return true
	}
	return false
}

func getData(k uint64) uint64 {
	return uint64(k) & DataMask
}

func setReadCount(k *uint64, c uint64) {
	rmReadCount(k)
	dummy := (0xF & c) << 60
	*k = (*k) | dummy
}

func setWrite(k *uint64, write bool) {
	rmWrite(k)
	var w uint64
	if write {
		w = 1
	}
	dummy := (0x1 & w) << 59
	*k = (*k) | dummy
}

func setTail(k *uint64, tail bool) {
	rmTail(k)
	var w uint64
	if tail {
		w = 1
	}
	dummy := (0x1 & w) << 58
	*k = (*k) | dummy
}

func setData(k *uint64, data uint64) {
	rmData(k)
	dummy := DataMask & data
	*k = (*k) | dummy
}

func rmReadCount(k *uint64) {
	v := (*k) & 0xFFF_FFFF_FFFF_FFFF
	*k = v
}

func rmWrite(k *uint64) {
	v := (*k) & 0xF7FF_FFFF_FFFF_FFFF
	*k = v
}

func rmTail(k *uint64) {
	v := (*k) & 0xFBFF_FFFF_FFFF_FFFF
	*k = v
}

func rmData(k *uint64) {
	v := (*k) & 0xFC00_0000_0000_0000
	*k = v
}

type item struct {
	Key   uint64
	Value atomic.Value
}

type table struct {
	Used    int64
	Buckets []item
}

func NewTable(size int) *table {
	t := &table{}
	t.Buckets = make([]item, size)
	return t
}

func (t *table) index(key uint64) int {
	data := getData(key)
	capacity := len(t.Buckets)
	return int(data % uint64(capacity))
}

func (t *table) Add(key uint64, x interface{}) error {
	idx := t.index(key)
	// wait write=0
	for {
		tmp := atomic.LoadUint64(&t.Buckets[idx].Key)
		if !getWrite(tmp) {
			if tmp > 0 && t.index(tmp) != idx {
				// This bucket occupy by other
				return ErrNoRoom
			}
			newr := tmp
			setWrite(&newr, true)
			if atomic.CompareAndSwapUint64(&t.Buckets[idx].Key, tmp, newr) {
				break
			}
		}
	}
	// wait read=0
	for {
		tmp := atomic.LoadUint64(&t.Buckets[idx].Key)
		if getReadCount(tmp) == 0 {
			break
		}
	}

	atomic.AddInt64(&t.Used, 1)
	return nil
}

func (t *table) Remove(key uint64) error {
	idx := t.index(key)

	atomic.AddInt64(&t.Used, -1)
	return nil
}
