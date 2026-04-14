#pragma once
static int mock_level_update_called = 0;
static inline void sodaLevelUpdate() { mock_level_update_called++; }
