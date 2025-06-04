// -- Replication of event Actions in C#, you can subscribe and unsubscribe to a event
// -- and you can invoke the event and all the listeners will be activated
#pragma once
#include <iostream>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

template<typename... Args>
class Event {
public:
	using Callback = std::function<void(Args...)>;

	using ListenerID = uint64_t;

	ListenerID Subscribe(Callback callback) {
		ListenerID id = _nextId++;
		_listeners[id] = callback;
		return id;
	}

	void UnSubscribe(ListenerID id) {
		_listeners.erase(id);
	}

	void Invoke(Args... args) {
		for (const auto& [id, callback] : _listeners) {
			if (callback)
				callback(args...);
		}
	}
	void UnsubscribeAll() {
		_listeners.clear();
	}
private:
	std::unordered_map<ListenerID, Callback> _listeners;
	ListenerID _nextId = 0;
};