package main

import (
	"fmt"
	"math/rand"
	"reflect"
	"sync"
	"time"
)

// TupleSpace represents a Linda tuple space
type TupleSpace struct {
	tuples [][]interface{}
	mutex  sync.RWMutex
	// Condition variables for blocking operations
	readWaiters  map[string][]chan []interface{}
	waitersLock  sync.Mutex
}

// NewTupleSpace creates a new tuple space
func NewTupleSpace() *TupleSpace {
	return &TupleSpace{
		tuples:      make([][]interface{}, 0),
		readWaiters: make(map[string][]chan []interface{}),
	}
}

// out writes a tuple to the space
func (ts *TupleSpace) out(tuple []interface{}) error {
	if len(tuple) == 0 {
		return fmt.Errorf("tuple cannot be empty")
	}

	// Validate tuple elements (only int64, float64, string allowed)
	for _, element := range tuple {
		switch element.(type) {
		case int64, float64, string:
			// Valid types
		default:
			return fmt.Errorf("unsupported type: %T", element)
		}
	}

	ts.mutex.Lock()
	ts.tuples = append(ts.tuples, tuple)
	ts.mutex.Unlock()

	// Notify waiting readers
	ts.notifyWaiters(tuple)

	return nil
}

// rd reads a tuple without removing it (blocking if no match found)
func (ts *TupleSpace) rd(pattern []interface{}) ([]interface{}, error) {
	if len(pattern) == 0 {
		return nil, fmt.Errorf("pattern cannot be empty")
	}

	for {
		ts.mutex.RLock()
		matches := ts.findMatches(pattern)
		if len(matches) > 0 {
			// Choose a random match
			selectedTuple := matches[rand.Intn(len(matches))]
			ts.mutex.RUnlock()
			return selectedTuple, nil
		}
		ts.mutex.RUnlock()

		// No matches found, wait for a matching tuple
		waitChan := ts.addWaiter(pattern)
		select {
		case matchedTuple := <-waitChan:
			return matchedTuple, nil
		case <-time.After(100 * time.Millisecond):
			// Continue the loop to check again
		}
	}
}

// in reads and removes a tuple (blocking if no match found)
func (ts *TupleSpace) in(pattern []interface{}) ([]interface{}, error) {
	if len(pattern) == 0 {
		return nil, fmt.Errorf("pattern cannot be empty")
	}

	for {
		ts.mutex.Lock()
		matchIndices := ts.findMatchIndices(pattern)
		if len(matchIndices) > 0 {
			// Choose a random match
			selectedIndex := matchIndices[rand.Intn(len(matchIndices))]
			selectedTuple := ts.tuples[selectedIndex]
			
			// Remove the tuple
			ts.tuples = append(ts.tuples[:selectedIndex], ts.tuples[selectedIndex+1:]...)
			ts.mutex.Unlock()
			return selectedTuple, nil
		}
		ts.mutex.Unlock()

		// No matches found, wait for a matching tuple
		waitChan := ts.addWaiter(pattern)
		select {
		case matchedTuple := <-waitChan:
			// Try to remove the tuple (it might have been taken by another client)
			return ts.in(pattern) // Recursive call to try removing again
		case <-time.After(100 * time.Millisecond):
			// Continue the loop to check again
		}
	}
}

// findMatches returns all tuples that match the given pattern
func (ts *TupleSpace) findMatches(pattern []interface{}) [][]interface{} {
	var matches [][]interface{}
	
	for _, tuple := range ts.tuples {
		if ts.matchesTuple(tuple, pattern) {
			matches = append(matches, tuple)
		}
	}
	
	return matches
}

// findMatchIndices returns indices of all tuples that match the given pattern
func (ts *TupleSpace) findMatchIndices(pattern []interface{}) []int {
	var indices []int
	
	for i, tuple := range ts.tuples {
		if ts.matchesTuple(tuple, pattern) {
			indices = append(indices, i)
		}
	}
	
	return indices
}

// matchesTuple checks if a tuple matches a pattern (nil acts as wildcard)
func (ts *TupleSpace) matchesTuple(tuple []interface{}, pattern []interface{}) bool {
	if len(tuple) != len(pattern) {
		return false
	}
	
	for i := 0; i < len(tuple); i++ {
		// nil in pattern acts as wildcard
		if pattern[i] != nil {
			if !reflect.DeepEqual(tuple[i], pattern[i]) {
				return false
			}
		}
	}
	
	return true
}

// addWaiter adds a waiter for a specific pattern
func (ts *TupleSpace) addWaiter(pattern []interface{}) chan []interface{} {
	ts.waitersLock.Lock()
	defer ts.waitersLock.Unlock()

	patternKey := ts.patternToString(pattern)
	waitChan := make(chan []interface{}, 1)
	
	if ts.readWaiters[patternKey] == nil {
		ts.readWaiters[patternKey] = make([]chan []interface{}, 0)
	}
	
	ts.readWaiters[patternKey] = append(ts.readWaiters[patternKey], waitChan)
	return waitChan
}

// notifyWaiters notifies all waiters that
