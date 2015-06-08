#ifndef SCREEN_TRANSITION_H
#define SCREEN_TRANSITION_H

#include "Screen.h"

#include "TemperatureManager.h"
#include "ViewManager.h"

namespace screen
{
	template <typename T>
		class ScreenTransition : public Screen , public Observer<T>
	{
		public:
			ScreenTransition(const char * title, const char * text, Subject<T> * model = 0);
			virtual ~ScreenTransition();

			void init();

			void draw();
			void press();

			void add(Screen & component);
			void update(T value);

		private:
			const char * m_text;

			T m_observed;

			Screen * m_back_screen;
			Screen * m_next_screen;
			uint8_t m_num_item_added;
	};

	template <typename T>
	ScreenTransition<T>::ScreenTransition(const char * title, const char * text, Subject<T> * model)
		: Screen(title, SELECTOR)
		, Observer<T>(model)
		, m_text(text)
		, m_num_item_added(0)
		, m_observed(0)
	{ }

	template <typename T>
	ScreenTransition<T>::~ScreenTransition()
	{ }

	template <typename T>
	void ScreenTransition<T>::init()
	{
		this->m_model->attach(this);
	}

	template <typename T>
	void ScreenTransition<T>::draw()
	{
		uint16_t target = (uint16_t) TemperatureManager::getInstance().getTargetTemperature();
		char c_target[4] = { 0 };
		snprintf(c_target, 4, "%d", target);

		char c_current[4] = { 0 };
		dtostrf(m_observed, 3, 0, c_current);

		painter.firstPage();
		do
		{
			painter.title(m_title);
			painter.box(m_text);

			uint8_t y_init = painter.coordinateYInit();
			uint8_t y_end = painter.coordinateYEnd();

			painter.setColorIndex(1);
			if(m_observed > 99.0)
			{
				painter.setPrintPos(31,(y_end + y_init)/2 - 9/2);
			}
			else if(m_observed > 9.0)
			{
				painter.setPrintPos(37,(y_end + y_init)/2 - 9/2);
			}
			else
			{
				painter.setPrintPos(43,(y_end + y_init)/2 - 9/2);
			}
			painter.print(c_current);
			painter.print(" / ");
			painter.print(c_target);
			
		} while ( painter.nextPage() );

		if (target == (uint16_t) m_observed)
		{
			ViewManager::getInstance().activeView(m_next_screen);
		}
	}

	template <typename T>
	void ScreenTransition<T>::press()
	{
		ViewManager::getInstance().activeView(m_back_screen);
	}

	template <typename T>
	void ScreenTransition<T>::add(Screen & component)
	{
		if (m_num_item_added % 2)
		{
			m_next_screen = &component;
		}
		else
		{
			m_back_screen = &component;
		}
		m_num_item_added++;
	}

	template<typename T>
	void ScreenTransition<T>::update(T value)
	{
		m_observed = value;
	}
}


#endif //SCREEN_TRANSITION_H