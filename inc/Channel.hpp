/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpinchas <rpinchas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:53:30 by rpinchas          #+#    #+#             */
/*   Updated: 2025/02/07 22:15:03 by rpinchas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include "../inc/Client.hpp"

class Channel {
	public:
		Channel(std::string& name, std::string& pass);
		~Channel();
	
		std::string getcName() const;
		std::string getcTopic() const;
		std::string getcPass() const;
		bool getChannelType() const;
		void setcName(const std::string& name);
		void setcTopic(const std::string& topic);
		void setcPass(const std::string& password);
		void setChannelType();

		void addClient(const Client *client);
		void addOperator(const Client *client);
		void rmClient(int fd);
		void rmOperator(int fd);

	private:
		int _fd;
		std::string _cName;
		std::string _cPass;
		std::string _cTopic;
		std::vector<Client*> _joined;
		std::vector<Client*> _operators;
		bool		_isInviteOnly;

};
