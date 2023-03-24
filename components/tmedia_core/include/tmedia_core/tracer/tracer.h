/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */
#ifndef CHROME_TRACE_H
#define CHROME_TRACE_H

#ifdef __linux__
#include <tmedia_config.h>
#endif

#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <map>
#include <optional>
#include <sstream>
#include <queue>
#include <condition_variable>
#include <mutex>

// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit#heading=h.puwqg050lyuy
#define TRACE_EVENT_PHASE_BEGIN         ('B')
#define TRACE_EVENT_PHASE_END           ('E')
#define TRACE_EVENT_PHASE_INSTANT       ('i')
#define TRACE_EVENT_PHASE_ASYNC_START   ('b')
#define TRACE_EVENT_PHASE_ASYNC_INSTANT ('n')
#define TRACE_EVENT_PHASE_ASYNC_END     ('e')
#define TRACE_EVENT_PHASE_METADATA      ('M')
#define TRACE_EVENT_PHASE_COUNTER       ('C')

struct TraceEventInfo
{
	std::string EventName;
	std::string Category;
	uint32_t ProcessID;
	uint32_t ThreadID;
	uint8_t EventType;
	uint32_t TimePoint;
	std::uintptr_t Id = 0; // used for async event
	std::map<std::string, std::string> Args;
};

class TraceEvent
{
public:
	template <typename... Arguments>
	TraceEvent *AddArgs(Arguments &&...args)
	{
		int v[] = {0, ((void)ForEach(std::forward<Arguments>(args)), 0)...};
		(void)v;
		return this;
	}

	void WriteInfo();

protected:
	TraceEventInfo m_info;

private:
	template <typename T>
	void ForEach(T t)
	{
		if (m_counter % 2 == 0) // it's a key, cache
		{
#if (__cplusplus >= 201703)
			if constexpr (std::is_same<T, std::string>::value)
#else
			if (std::is_same<T, std::string>::value)
#endif
			{
				m_currentKey = t;
			}

#if (__cplusplus >= 201703)
			if constexpr (std::is_same<T, const char *>::value)
#else
			if (std::is_same<T, const char *>::value)
#endif
			{
				m_currentKey = t;
			}
		}
		else // it's a value, push
		{
			std::stringstream ss;
			ss << t;
			m_info.Args[m_currentKey] = ss.str();
		}
		m_counter++;
	}
	int m_counter = 0;
	std::string m_currentKey;
};

class DurationEvent : public TraceEvent
{
public:
	DurationEvent(const std::string &category, const std::string &name);
	~DurationEvent();
};

class InstantEvent : public TraceEvent
{
public:
	InstantEvent(const std::string &category, const std::string &name);
	~InstantEvent();
};

class CounterEvent : public TraceEvent
{
public:
	CounterEvent(const std::string &category, const std::string &name, int32_t val = 0);
	~CounterEvent();
};

class AsyncEvent : public TraceEvent
{
public:
	AsyncEvent(const std::string &category, const std::string &name);
	void InstantAsyncEvent(const std::string &category, const std::string &name);
	~AsyncEvent();
};

class MetaProcessEvent : public TraceEvent
{
public:
	MetaProcessEvent(const std::string &name);
	~MetaProcessEvent();
};

class MetaThreadEvent : public TraceEvent
{
public:
	MetaThreadEvent(const std::string &name);
	~MetaThreadEvent();
};

class Tracer
{
public:
	static Tracer &Instance();
	~Tracer();
	void StartSession(const std::string &sessionName = "trace.json", bool useInfoConsoleLogs = false);
	void EndSession();
	void WriteInfo(const TraceEventInfo &info);

	DurationEvent *BeginDurationEvent(const std::string &category, const std::string &name);
	void EndDurationEvent(const std::string &category, const std::string &name);
	DurationEvent *GetDurationEvent(const std::string &category, const std::string &name);

	AsyncEvent *StartAsyncEvent(const std::string &category, const std::string &name);
	AsyncEvent *InstantAsyncEvent(const std::string &category, const std::string &name);
	void EndAsyncEvent(const std::string &category, const std::string &name);
	AsyncEvent *GetAsyncEvent(const std::string &category, const std::string &name);

private:
	static std::unique_ptr<Tracer> m_instance;

	std::map<std::string, DurationEvent *> m_DurationEvents;
	std::map<std::string, AsyncEvent *> m_AsyncEvents;
	void ThreadJob(std::string sessionName);

	bool m_isSessionActive;
	bool m_writeComma = false;
	bool m_threadRunning = false;
	bool m_useInternalCommandLogs = false;
	std::unique_ptr<std::thread> m_thread;	 // Writing thread
	std::queue<TraceEventInfo> m_eventQueue; // List of events to write
	std::mutex m_outstreamMutex;			 // Used to lock list of events to write
	std::mutex m_asyncEventMapMutex;		 // Used to lock the map of async events
	std::condition_variable m_waitCondition; // Synchronize the list of events};
};

#ifdef _MSC_VER
#define __FUNC_STR__ __FUNCSIG__
#else
#define __FUNC_STR__ __PRETTY_FUNCTION__
#endif

#ifdef CONFIG_TMEDIA_TRACE
#define TRACE_BEGIN(fileName) Tracer::Instance().StartSession(fileName)
#define TRACE_BEGIN_WLOGS(fileName) Tracer::Instance().StartSession(fileName, true)
#define TRACE_END() Tracer::Instance().EndSession()
#define TRACE_FUNC(...)                                     \
	DurationEvent __event__(__FILE__, __FUNC_STR__); \
	__event__.AddArgs(__VA_ARGS__);                         \
	__event__.WriteInfo()
#define TRACE_SCOPE(category, name, ...)     \
	DurationEvent __event__(category, name); \
	__event__.AddArgs(__VA_ARGS__);          \
	__event__.WriteInfo()
#define TRACE_DURATION_BEGIN(category, name, ...) Tracer::Instance().BeginDurationEvent(category, name)->AddArgs(__VA_ARGS__)->WriteInfo()
#define TRACE_DURATION_END(category, name, ...)                                    \
	{                                                                              \
		Tracer::Instance().GetDurationEvent(category, name)->AddArgs(__VA_ARGS__); \
		Tracer::Instance().EndDurationEvent(category, name);                       \
	}
#define TRACE_ASYNC_START(category, name, ...) Tracer::Instance().StartAsyncEvent(category, name)->AddArgs(__VA_ARGS__)->WriteInfo()
#define TRACE_ASYNC_INSTANT(category, name, ...) Tracer::Instance().InstantAsyncEvent(category, name)->AddArgs(__VA_ARGS__)->WriteInfo()
#define TRACE_ASYNC_END(category, name, ...)                                    \
	{                                                                           \
		Tracer::Instance().GetAsyncEvent(category, name)->AddArgs(__VA_ARGS__); \
		Tracer::Instance().EndAsyncEvent(category, name);                       \
	}
#define TRACE_INSTANT(category, name, ...)      \
	{                                           \
		InstantEvent __event__(category, name); \
		__event__.AddArgs(__VA_ARGS__);         \
	}
#define TRACE_COUNTER(category, name, val)           \
	{                                                \
		CounterEvent __event__(category, name, val); \
	}
#define TRACE_META_PROCESS(process_name)          \
	{                                             \
		MetaProcessEvent __event__(process_name); \
	}
#define TRACE_META_THREAD(thread_name)          \
	{                                           \
		MetaThreadEvent __event__(thread_name); \
	}
#else
#define TRACE_BEGIN(fileName)
#define TRACE_BEGIN_WLOGS(fileName)
#define TRACE_END()
#define TRACE_FUNC(...)
#define TRACE_SCOPE(category, name, ...)
#define TRACE_DURATION_BEGIN(category, name, ...)
#define TRACE_DURATION_END(category, name, ...)
#define TRACE_ASYNC_START(category, name, ...)
#define TRACE_ASYNC_INSTANT(category, name, ...)
#define TRACE_ASYNC_END(category, name, ...)
#define TRACE_INSTANT(category, name, ...)
#define TRACE_COUNTER(category, name, val)
#define TRACE_META_PROCESS(process_name)
#define TRACE_META_THREAD(thread_name)
#endif

#endif /* CHROME_TRACE_H */
