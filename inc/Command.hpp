/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dvaisman <dvaisman@student.42vienna.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 13:11:39 by dvaisman          #+#    #+#             */
/*   Updated: 2025/02/10 13:14:28 by dvaisman         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class Command
{
	public:
		Command();
		Command(const Command &src);
		void executeCommand(int fd, const std::string &command);
	private:
		void commandNick(int fd, const std::string &command);
		void commandUser(int fd, const std::string &command);
		void commandJoin(int fd, const std::string &command);
};