<<<<<<< HEAD
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dkohn <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/07 12:45:51 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/14 20:26:50 by dkohn            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

=======
>>>>>>> origin/master
#pragma once

#include <string>
#include "Server.hpp"

class Server;

class Command
{
	public:
		// Constructors
		Command();
		Command(const Command &src);
		Command &operator=(const Command &src);
		~Command();

		// Methods
		void executeCommand(Server &server, int fd, const std::string &command);
	private:
		// Methods
		void commandNick(Server &server, int fd, const std::string &command);
		void commandUser(Server &server, int fd, const std::string &command);
		void commandJoin(Server &server, int fd, const std::string &command);
		void commandMode(Server &server, int fd, const std::string &command);
		void commandPass(Server &server, int fd, const std::string &command);
		void commandPing(int fd, const std::string &command);
		void commandCap(int fd, const std::string &command);
};
