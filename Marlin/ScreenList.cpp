#include "ScreenList.h"
#include "cardreader.h"

State_t state;
Event_t event;

State_func_t * const state_table[NUM_STATES] =
{
	do_state_prepare,
	do_state_paint
};

State_t run_state(State_t current_state, Event_t event)
{
	return state_table[current_state](event);
};

State_t do_state_prepare(Event_t event)
{
	SERIAL_ECHOLN ("[do_state_prepare]");
	if (event == EVENT_PREPARED)
		return STATE_PAINT;

	return STATE_PREPARE;
}

State_t do_state_paint(Event_t event)
{
	SERIAL_ECHOLN ("[do_state_paint]");
	if (event == EVENT_KEYPRESS)
		return STATE_PREPARE;

	return STATE_PAINT;
}


namespace screen
{
	ScreenList::ScreenList(const char * title)
		: Screen(title, LIST)
		, m_index(0)
		, m_icon_index(0)
		, m_num_list(0)
		, m_offset(0)
		, m_num_item_added(0)
	{
		memset(m_directory, 0, sizeof(m_directory));
		m_directory_is_root = false;
		state = STATE_PREPARE;
	}

	ScreenList::~ScreenList()
	{
	}

	void ScreenList::left()
	{
		if (m_index == 0)
		{
			m_index = 0;
		}
		else
		{
			--m_index;
		}
	}

	void ScreenList::right()
	{
		if ( m_index == (m_num_list -1) )
		{
			m_index = m_num_list -1;
		}
		else
		{
			++m_index;
		}
	}

	void ScreenList::draw()
	{
		if (sdcardChanged())
		{
			if (m_sdcard_inserted)
			{
				card.initsd();
			}
			else
			{
				card.release();
			}
			state = run_state(state, EVENT_KEYPRESS);
		}

		if (state == STATE_PREPARE)
		{
			m_num_list = card.getnrfilenames();
			m_index = 0;

			card.getWorkDirName();
			strncpy(m_directory, card.filename, 9);
			m_directory[9] = '\0';

			if (card.filename[0] != '/') {
				m_directory_is_root = false;
				m_offset = 2;
			}
			else
			{
				m_directory_is_root = true;
				m_offset = 1;
			}

			m_num_list += m_offset;

			m_scroll_size = (float) 47 / m_num_list;

			state = run_state(state, EVENT_PREPARED);
		}

		painter.firstPage();
		do
		{
			// Draw title
			if (m_directory_is_root == true)
			{
				painter.title(m_title);
			}
			else
			{
				uint8_t x_init = painter.coordinateXInit();
				uint8_t y_init = painter.coordinateYInit();
				uint8_t x_end = painter.coordinateXEnd();

				painter.setColorIndex(1);
				painter.setFont(u8g_font_6x9);
				painter.setPrintPos(x_init, y_init + 3);
				painter.print(m_directory);
				painter.drawLine(x_init, y_init + 13, x_end, y_init + 13);

				painter.coordinateYInit(14);
			}

			// Draw list
			uint8_t window_size = 50 / (max_font_height + 1);
			uint8_t window_selector = window_size / 2;
			if (window_size % 2 == 0)
			{
				window_selector--;
			}

			for (uint8_t i = 0; i < window_size; i++)
			{
				painter.setPrintPos(painter.coordinateXInit(), painter.coordinateYInit() + i * (max_font_height + 1));

				if (i == window_selector) {
					painter.setColorIndex(1);
					painter.drawBox(painter.coordinateXInit(), painter.coordinateYInit() + i * (max_font_height + 1), 128, max_font_height);
					painter.setColorIndex(0);
				}
				else
				{
					painter.setColorIndex(1);
				}


				if ((int)(m_index + i - window_selector) < 0)
				{
					continue;
				}

				if ((m_index + i - window_selector) == 0)
				{
					painter.drawBitmap(painter.coordinateXInit(), painter.coordinateYInit() + i * (max_font_height + 1), little_icon_width, little_icon_height, bits_back_small);
					painter.setPrintPos(painter.coordinateXInit() + 9, painter.coordinateYInit() + i * (max_font_height + 1));
					painter.print("Back to main menu");
				}

				if (m_directory_is_root == false && (m_index + i - window_selector) == 1)
				{
					painter.drawBitmap(painter.coordinateXInit(), painter.coordinateYInit() + i * (max_font_height + 1), little_icon_width, little_icon_height, bits_updir_small);
					painter.setPrintPos(painter.coordinateXInit() + 9, painter.coordinateYInit() + i * (max_font_height + 1));
					painter.print("Previous folder");
				}
				else
				{
					card.getfilename(m_index + i - window_selector - m_offset);

					SERIAL_ECHO(m_title);
					SERIAL_ECHO(" ");
					SERIAL_ECHO(m_directory);
					SERIAL_ECHO("  (item ");
					SERIAL_ECHO(m_index + 1);
					SERIAL_ECHO(" / ");
					SERIAL_ECHO(m_num_list);
					SERIAL_ECHO(") : ");
					SERIAL_ECHO(card.longFilename);

					if (card.filenameIsDir == true)
					{
						painter.drawBitmap(painter.coordinateXInit(), painter.coordinateYInit() + i * (max_font_height + 1), little_icon_width, little_icon_height, bits_folder_small);
						painter.setPrintPos(painter.coordinateXInit() + 9, painter.coordinateYInit() + i * (max_font_height + 1));
						SERIAL_ECHOLN(" [D]");
					}
					else
					{
						SERIAL_ECHOLN("");
					}

					painter.print(card.longFilename);
				}
			}

			// Draw scroll bar
			painter.setColorIndex(1);
			painter.drawBox(122, 13, 6, 51);
			painter.setColorIndex(0);
			painter.drawBox(123, 14, 4, 49);

			int8_t scroll_bottom_bar = (m_index + 1) * m_scroll_size;
			if (scroll_bottom_bar < 1)
			{
				scroll_bottom_bar = 1;
			}
			painter.setColorIndex(1);
			painter.drawBox(124, 15, 2, scroll_bottom_bar);

			int8_t scroll_upper_bar = scroll_bottom_bar - m_scroll_size;
			if (scroll_upper_bar > 0)
			{
				painter.setColorIndex(0);
				painter.drawBox(124, 15, 2, scroll_upper_bar);
			}

		} while( painter.nextPage() );
	}

	Screen & ScreenList::press(Screen * parent_view)
	{
		SERIAL_ECHOLN("ScreenList::press");
		state = run_state(state, EVENT_KEYPRESS);
		SERIAL_ECHOLN("ScreenList::press - State machine run");

		if (m_index == 0)
		{
			return * m_back_screen;
		}

		if (m_directory_is_root == false && (m_index == 1))
		{
			card.updir();
			return * this;
		}
		else
		{
		/*
			card.getfilename(m_index);
			SERIAL_ECHOLN("ScreenList::press - card.getfilename");
			*/
			card.getfilename(m_index - m_offset);
			if (card.filenameIsDir == true)
			{
				SERIAL_ECHOLN("ScreenList::press - isDir");
				card.chdir(card.filename);
				return * this;
			}
			SERIAL_ECHOLN("ScreenList::press - return next screen");
			return * m_next_screen;
		}
	}

	void ScreenList::add(Screen & component)
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

	void ScreenList::icon(Icon & component)
	{
		if (m_icon_index < 1)
		{
			m_icon = &component;
		}
      else
      {
         m_icon_alternate = &component;
      }
      ++m_icon_index;
	}

	Icon & ScreenList::icon()
	{
		if (sdcardChanged())
		{
			if (m_sdcard_inserted)
			{
				card.initsd();
			}
			else
			{
				card.release();
			}
		}

		if (m_sdcard_inserted == true)
		{
			return * m_icon_alternate;
		}
		return * m_icon;
	}

	bool ScreenList::sdcardChanged()
	{
		// Check if the SD card has been inserted/removed.
		if (m_sdcard_inserted != IS_SD_INSERTED)
		{
			m_sdcard_inserted = IS_SD_INSERTED;
			return true;
		}

		return false;
	}
}
