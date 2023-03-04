//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "type.h"
#include <set>
#include <deque>
#include <queue>

class EventQueue {
public:
	class Event {
	public:
		Index pin = -1;
		//was the pin set externaly
		bool external = false;
		int64_t time = 0;
		int64_t insertIndex = 0;

		bool operator<(const Event& e) const {
			if (time == e.time) {
				return insertIndex > e.insertIndex;
			}
			else {
				return time > e.time;
			}
		}
	};
	std::deque<Event> updateQueue;
	std::set<Index> updateSet;
	std::priority_queue<Event> sortedUpdateQueue;
	int64_t nextInsertIndex = 0;
	bool useUpdateSet = false;
	bool sortQueue = false;

	void add(Index pin, int64_t time, bool external) {
		if (useUpdateSet) {
			if (!updateSet.contains(pin)) {
				if (sortQueue) {
					sortedUpdateQueue.push({ pin, external, time, nextInsertIndex++ });
				}
				else {
					updateQueue.push_back({ pin, external, time });
				}
				updateSet.insert(pin);
			}
		}
		else {
			if (sortQueue) {
				sortedUpdateQueue.push({ pin, external, time, nextInsertIndex++ });
			}
			else {
				updateQueue.push_back({ pin, external, time });
			}
		}
	}

	Event get() {
		if (sortQueue) {
			return sortedUpdateQueue.top();
		}
		else {
			return updateQueue.front();
		}
	}

	void pop() {
		if (useUpdateSet) {
			updateSet.erase(get().pin);
		}
		if (sortQueue) {
			sortedUpdateQueue.pop();
		}
		else {
			updateQueue.pop_front();
		}
	}

	bool empty() {
		if (sortQueue) {
			return sortedUpdateQueue.empty();
		}
		else {
			return updateQueue.empty();
		}
	}
};
