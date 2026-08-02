module;
#include <MiniFB_cpp.h>

export module minifb;

import std;

struct MfbWindowDeleter {
	void operator()(struct mfb_window* window) const {
		if (window) {
			mfb_close(window);
		}
	}
};

export using mfb_window_ = std::unique_ptr<struct mfb_window, MfbWindowDeleter>;

export using ::mfb_open_ex;
export using ::mfb_update_ex;
export using ::mfb_wait_sync;
export using ::mfb_close;
export using ::mfb_get_key_buffer;

export using::mfb_window_flags; 
export using::mfb_update_state; 
export using::mfb_key;