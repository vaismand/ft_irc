/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 12:45:51 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/07 12:47:45 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "Server.hpp"

class Server;

class Command
{
	public:
		Command();
		Command(const Command &src);
		void executeCommand(Server &server, int fd, const std::string &command);
	private:
		void commandNick(Server &server, int fd, const std::string &command);
		void commandUser(Server &server, int fd, const std::string &command);
		void commandJoin(Server &server, int fd, const std::string &command);
		void commandPing(Server &server, int fd, const std::string &command);
};
