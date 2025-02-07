/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rpinchas <rpinchas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:53:30 by rpinchas          #+#    #+#             */
/*   Updated: 2025/02/06 15:47:31 by rpinchas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

class Channel {
	public:
		Channel();
		~Channel();
	
		std::string getcName() const;
		std::string getcTopic() const;
		std::string getcPass() const;
		bool getChannelType() const;
		void setcName(const std::string& name);
		void setcTopic(const std::string& topic);
		void setcPass(const std::string& password);
		void setChannelType();

	private:
		int _fd;
		std::string _cName;
		std::string _cTopic;
		std::string _cPass;
		std::vector<int> _joined;
		bool		_isInviteOnly;

};
