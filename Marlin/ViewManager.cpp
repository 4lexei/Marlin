#include "ViewManager.h"

namespace screen
{
	ViewManager & ViewManager::getInstance()
	{
		static ViewManager instance;
		return instance;
	}

	void ViewManager::activeView(Screen* screen)
	{
		m_active_view = screen;
		m_active_view->init();
	}

	Screen* ViewManager::activeView()
	{
		return m_active_view;
	}

	ViewManager::ViewManager()
		: m_active_view(NULL)
	{ }

	ViewManager::~ViewManager()
	{ }
}