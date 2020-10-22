/*
 * Copyright 2013-2020 Automatak, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Automatak
 * LLC (www.automatak.com) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Automatak LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include "channel/UDPSocketChannel.h"

namespace opendnp3
{

UDPSocketChannel::UDPSocketChannel(const std::shared_ptr<exe4cpp::StrandExecutor>& executor,
                                   asio::ip::udp::socket socket,
                                   asio::ip::udp::endpoint remoteEndpoint)
    : IAsyncChannel(executor), socket(std::move(socket)), remoteEndpoint(remoteEndpoint)
{
}

void UDPSocketChannel::BeginReadImpl(ser4cpp::wseq_t dest)
{
    auto senderEndpoint = std::make_shared<asio::ip::udp::endpoint>();

    auto callback = [this, dest, senderEndpoint](const std::error_code& ec, size_t num) {
        if (*senderEndpoint == this->remoteEndpoint) {
          this->OnReadCallback(ec, num);
        } else {
          this->BeginReadImpl(dest);
        }
    };

    socket.async_receive_from(asio::buffer(dest, dest.length()), *senderEndpoint, this->executor->wrap(callback));
}

void UDPSocketChannel::BeginWriteImpl(const ser4cpp::rseq_t& buffer)
{
    auto callback = [this](const std::error_code& ec, size_t num) { this->OnWriteCallback(ec, num); };

    socket.async_send_to(asio::buffer(buffer, buffer.length()), this->remoteEndpoint, this->executor->wrap(callback));
}

void UDPSocketChannel::ShutdownImpl()
{
    std::error_code ec;
    socket.shutdown(asio::socket_base::shutdown_type::shutdown_both, ec);
    socket.close(ec);
}

} // namespace opendnp3
