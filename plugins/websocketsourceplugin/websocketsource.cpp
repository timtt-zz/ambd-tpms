/*
	Copyright (C) 2012  Intel Corporation

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "websocketsource.h"
#include <iostream>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <glib.h>
#include <sstream>
#include <json-glib/json-glib.h>
#include <listplusplus.h>
#include "debugout.h"
#define __SMALLFILE__ std::string(__FILE__).substr(std::string(__FILE__).rfind("/")+1)
libwebsocket_context *context;
WebSocketSource *source;
AbstractRoutingEngine *m_re;

static int callback_http_only(libwebsocket_context *context,struct libwebsocket *wsi,enum libwebsocket_callback_reasons reason,void *user, void *in, size_t len);
static struct libwebsocket_protocols protocols[] = {
	{
		"http-only",
		callback_http_only,
		0,
	},
	{  /* end of list */
		NULL,
		NULL,
		0
	}
};

//Called when a client connects, subscribes, or unsubscribes.
void WebSocketSource::checkSubscriptions()
{
	PropertyList notSupportedList;
	while (queuedRequests.size() > 0)
	{
		VehicleProperty::Property prop = queuedRequests.front();
		queuedRequests.pop_front();
		if (ListPlusPlus<VehicleProperty::Property>(&activeRequests).contains(prop))
		{
			return;
		}
		activeRequests.push_back(prop);
		stringstream s;
		s << "{\"type\":\"method\",\"name\":\"subscribe\",\"data\":[\"" << prop << "\"],\"transactionid\":\"" << "d293f670-f0b3-11e1-aff1-0800200c9a66" << "\"}";

		string replystr = s.str();
		DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Reply:" << replystr << "\n";
		//printf("Reply: %s\n",replystr.c_str());

		char *new_response = new char[LWS_SEND_BUFFER_PRE_PADDING + strlen(replystr.c_str()) + LWS_SEND_BUFFER_POST_PADDING];
		new_response+=LWS_SEND_BUFFER_PRE_PADDING;
		strcpy(new_response,replystr.c_str());
		libwebsocket_write(clientsocket, (unsigned char*)new_response, strlen(new_response), LWS_WRITE_TEXT);
		delete (char*)(new_response-LWS_SEND_BUFFER_PRE_PADDING);
	}
}
void WebSocketSource::setConfiguration(map<string, string> config)
{
	//printf("WebSocketSource::setConfiguration has been called\n");
	std::string ip;
	int port;
	configuration = config;
	for (map<string,string>::iterator i=configuration.begin();i!=configuration.end();i++)
	{
		DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Incoming setting for WebSocketSource:" << (*i).first << ":" << (*i).second << "\n";
		//printf("Incoming setting: %s:%s\n",(*i).first.c_str(),(*i).second.c_str());
		if ((*i).first == "ip")
		{
			ip = (*i).second;
		}
		if ((*i).first == "port")
		{
			port = boost::lexical_cast<int>((*i).second);
		}
	}
	//printf("Connecting to websocket server at %s port %i\n",ip.c_str(),port);
	DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Connecting to websocket server at" << ip << ":" << port << "\n";
	clientsocket = libwebsocket_client_connect(context, ip.c_str(), port, 0,"/", "localhost", "websocket",protocols[0].name, -1);
	
}
bool gioPollingFunc(GIOChannel *source,GIOCondition condition,gpointer data)
{
	//This is the polling function. If it return false, glib will stop polling this FD.
	//printf("Polling...%i\n",condition);
	lws_tokens token;
	struct pollfd pollstruct;
	int newfd = g_io_channel_unix_get_fd(source);
	pollstruct.fd = newfd;
	pollstruct.events = condition;
	pollstruct.revents = condition;
	libwebsocket_service_fd(context,&pollstruct);
	if (condition == G_IO_HUP)
	{
		//Hang up. Returning false closes out the GIOChannel.
		//printf("Callback on G_IO_HUP\n");
		return false;
	}
	if (condition == G_IO_IN)
	{
	}
	
	return true;
}

static int callback_http_only(libwebsocket_context *context,struct libwebsocket *wsi,enum libwebsocket_callback_reasons reason,void *user, void *in, size_t len)
{
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 4096 + LWS_SEND_BUFFER_POST_PADDING];
	int l;
	switch (reason)
	{
		case LWS_CALLBACK_CLOSED:
			//fprintf(stderr, "mirror: LWS_CALLBACK_CLOSED\n");
			//wsi_mirror = NULL;
			//printf("Connection closed!\n");
			break;

		case LWS_CALLBACK_CLIENT_ESTABLISHED:
		{
			//This happens when a client initally connects. We need to request the support event types.
			source->clientConnected = true;
			source->checkSubscriptions();
			//printf("Incoming connection!\n");
			DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Incoming connection" << "\n";
			stringstream s;
			s << "{\"type\":\"method\",\"name\":\"getSupportedEventTypes\",\"data\":[],\"transactionid\":\"" << "d293f670-f0b3-11e1-aff1-0800200c9a66" << "\"}";
		
			string replystr = s.str();
			DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Reply:" << replystr << "\n";
			char *new_response = new char[LWS_SEND_BUFFER_PRE_PADDING + strlen(replystr.c_str()) + LWS_SEND_BUFFER_POST_PADDING];
			new_response+=LWS_SEND_BUFFER_PRE_PADDING;
			strcpy(new_response,replystr.c_str());
			libwebsocket_write(wsi, (unsigned char*)(new_response), strlen(new_response), LWS_WRITE_TEXT);  
			delete (char*)(new_response-LWS_SEND_BUFFER_PRE_PADDING);

			break;
		}
		case LWS_CALLBACK_CLIENT_RECEIVE:
		{
			//Incoming JSON reqest.
			GError* error = nullptr;
			JsonParser* parser = json_parser_new();
			if (!json_parser_load_from_data(parser,(char*)in,len,&error))
			{
				DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Error loading JSON\n";
				return 0;
			}
			
			JsonNode* node = json_parser_get_root(parser);
			if(node == nullptr)
			{
				DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Error getting root node of json\n";
				//throw std::runtime_error("Unable to get JSON root object");
				return 0;
			}
			
			JsonReader* reader = json_reader_new(node);
			if(reader == nullptr)
			{
				DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "json_reader is null!\n";
				//throw std::runtime_error("Unable to create JSON reader");
				return 0;
			}
			
			
			
			
			
			string type;
			json_reader_read_member(reader,"type");
			type = json_reader_get_string_value(reader);
			json_reader_end_member(reader);
			
			string  name;
			json_reader_read_member(reader,"name");
			name = json_reader_get_string_value(reader);
			json_reader_end_member(reader);

			list<string> data;
			json_reader_read_member(reader,"data");
			if (json_reader_is_array(reader))
			{
				for(int i=0; i < json_reader_count_elements(reader); i++)
				{
					json_reader_read_element(reader,i);
					string path = json_reader_get_string_value(reader);
					data.push_back(path);
					json_reader_end_element(reader);
				}
			}
			else
			{
				string path = json_reader_get_string_value(reader);
				if (path != "")
				{
					data.push_back(path);
				}
			}
			json_reader_end_member(reader);
			
			string id;
			json_reader_read_member(reader,"transactionid");
			if (strcmp("gchararray",g_type_name(json_node_get_value_type(json_reader_get_value(reader)))) == 0)
			{
				//Type is a string
				id = json_reader_get_string_value(reader);
			}
			else
			{
				//Type is an integer
				stringstream strstr;
				strstr << json_reader_get_int_value(reader);
				id = strstr.str();
			}
			json_reader_end_member(reader);
			
			///TODO: this will probably explode:
			//mlc: I agree with Kevron here, it does explode.
			//if(error) g_error_free(error);
			
			g_object_unref(reader);
			g_object_unref(parser);
			
			
			if (type == "valuechanged")
			{
				//printf("Value changed: %s, %s\n",name.c_str(),data.front().c_str());
				DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Value changed:" << name << data.front() << "\n";
				//Name should be a valid property
				//	routingEngine->updateProperty(VehicleProperty::VehicleSpeed, velocity);
				//data.front()
				try
				{
					AbstractPropertyType* type = VehicleProperty::getPropertyTypeForPropertyNameValue(name,data.front());
					m_re->updateProperty(name, type);
					delete type;
				}
				catch (exception ex)
				{
					//printf("Exception %s\n",ex.what());
					DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Exception:" << ex.what() << "\n";
				}
				//printf("Done\n");
				/*if (name == "get")
				{
					if (data.size() > 0)
					{
					}
				}*/
			}
			else if (type == "methodReply")
			{
				if (name == "getSupportedEventTypes")
				{
					//printf("Got supported events!\n");
					DebugOut() << __SMALLFILE__ <<":"<< __LINE__ << "Got getSupportedEventTypes request\n";
					PropertyList props;
					while (data.size() > 0)
					{
						string val = data.front();
						data.pop_front();	
						props.push_back(val);
						
					}
					source->setSupported(props);
					//m_re->updateSupported(m_supportedProperties,PropertyList());
				}
			}
			break;
		}
		case LWS_CALLBACK_ADD_POLL_FD:
		{
			//Add a FD to the poll list.
			GIOChannel *chan = g_io_channel_unix_new((int)(long)user);
			g_io_add_watch(chan,G_IO_IN,(GIOFunc)gioPollingFunc,0);
			g_io_add_watch(chan,G_IO_PRI,(GIOFunc)gioPollingFunc,0);
			break;
		}
		return 0;
	}
}
void WebSocketSource::setSupported(PropertyList list)
{
	m_supportedProperties = list;
	m_re->updateSupported(list,PropertyList());
}

WebSocketSource::WebSocketSource(AbstractRoutingEngine *re, map<string, string> config) : AbstractSource(re, config)
{
	clientConnected = false;
	source = this;
	m_re = re;  
	context = libwebsocket_create_context(CONTEXT_PORT_NO_LISTEN, NULL,protocols, NULL, NULL, -1, -1, 0);
	
	
	re->setSupported(supported(), this);
	//printf("websocketsource loaded!!!\n");
	
}
PropertyList WebSocketSource::supported()
{
	return m_supportedProperties;
}
extern "C" AbstractSource * create(AbstractRoutingEngine* routingengine, map<string, string> config)
{
	return new WebSocketSource(routingengine, config);
	
}
string WebSocketSource::uuid()
{
	return "d293f670-f0b3-11e1-aff1-0800200c9a66";
}
boost::any WebSocketSource::getProperty(VehicleProperty::Property property)
{
	if(property == VehicleProperty::VehicleSpeed)
	{
		//return velocity;
	}
	else if(property == VehicleProperty::EngineSpeed)
	{
		//return engineSpeed;
	}
}
void WebSocketSource::subscribeToPropertyChanges(VehicleProperty::Property property)
{
	//printf("Subscribed to property: %s\n",property.c_str());
	queuedRequests.push_back(property);
	if (clientConnected)
	{
		checkSubscriptions();
	}
}


void WebSocketSource::unsubscribeToPropertyChanges(VehicleProperty::Property property)
{
	removeRequests.push_back(property);
	if (clientConnected)
	{
		checkSubscriptions();
	}
}


void WebSocketSource::getPropertyAsync(AsyncPropertyReply *reply)
{
	/*if(reply->property == VehicleProperty::VehicleSpeed)
	{
		reply->value = velocity;
		reply->completed(reply);
	}
	else if(reply->property == VehicleProperty::EngineSpeed)
	{
		reply->value = engineSpeed;
		reply->completed(reply);
	}
	else if(reply->property == VehicleProperty::AccelerationX)
	{
		reply->value = accelerationX;
		reply->completed(reply);
	}
	else if(reply->property == VehicleProperty::TransmissionShiftPosition)
	{
		reply->value = transmissionShiftPostion;
		reply->completed(reply);
	}
	else if(reply->property == VehicleProperty::SteeringWheelAngle)
	{
		reply->value = steeringWheelAngle;
		reply->completed(reply);
	}*/
}

void WebSocketSource::setProperty(VehicleProperty::Property , AbstractPropertyType * )
{

}
