#pragma once
#include<memory>
#include<functional>
class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnectionPtr=std::shared_ptr<TcpConnection>;// the RAII style of memory manager
// the const TcpConnectionPtr& is ...
using ConnectionCallback=std::function<void(const TcpConnectionPtr&)>;
using CloseCallback=std::function<void (const TcpConnectionPtr&)>;
using WriteCompleteCallback=std::function<void(const TcpConnectionPtr&)>;
using MessageCallback=std::function<void(const TcpConnectionPtr&,Buffer*,Timestamp)>;
//the following type,emm I don't know this.
using HighWaterMarkCallback=std::function<void(const TcpConnectionPtr&,size_t)>;
