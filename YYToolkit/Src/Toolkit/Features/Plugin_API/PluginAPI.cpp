#include "PluginAPI.hpp"

const PluginState& YYTK_CreateState()
{
	PluginState State;
	Tool::API::PluginStates.push_back(State);

	State.m_nID = Tool::API::PluginStates.size() + 1;

	return Tool::API::PluginStates.back();
}