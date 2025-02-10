/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 13:13:43 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/10 13:16:10 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Command.hpp"

Command::Command() {}

Command::Command(const Command &src) {
	(void)src;
}

void Command::commandNick(int fd, const std::string &command)
{
	(void)fd;
	(void)command;
}

void Command::commandUser(int fd, const std::string &command)
{
	(void)fd;
	(void)command;
}

void Command::commandJoin(int fd, const std::string &command)
{
	(void)fd;
	(void)command;
}

void Command::executeCommand(int fd, const std::string &command)
{
	if (command.find("NICK") == 0)
		commandNick(fd, command);
	else if (command.find("USER") == 0)
		commandUser(fd, command);
	else if (command.find("JOIN") == 0)
		commandJoin(fd, command);
}
