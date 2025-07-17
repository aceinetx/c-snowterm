#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
	int y, x;
} Vec2;

typedef struct {
	Vec2 pos;
	char ch;
} Snowflake;

typedef struct {
	char ch;
	float v;
} SnowflakeChar;

Snowflake* g_snowflakes;
size_t g_snowflakes_size;
size_t g_snowflakes_count;
SnowflakeChar g_snowflake_charmap[] = {{'*', 1.0f}, {'+', 0.8f}, {'.', 0.4f}};
const size_t g_snowflake_charmap_len = 3;

#ifdef _WIN32
#include <windows.h>
#else
void usleep(long);
#endif

static void sleep_seconds(float seconds) {
	if (seconds < 0) {
		return;
	}

#ifdef _WIN32
	Sleep((DWORD)(seconds * 1000));
#else
	usleep((long)(seconds * 1000000));
#endif
}

static float random_float(void) {
	return (float)rand() / (float)RAND_MAX;
}

static Vec2 max_dimensions(void) {
	static Vec2 dimensions = {0, 0};
	if (dimensions.x == 0 || dimensions.y == 0) {
		dimensions.y = getmaxy(stdscr) - 1;
		dimensions.x = getmaxx(stdscr) - 2;
	}
	return dimensions;
}

static size_t snowflakes_len(void) {
	size_t len = 0;
	size_t i;
	for (i = 0; i < g_snowflakes_count; i++) {
		if (g_snowflakes[i].ch == 0)
			break;
		len++;
	}
	return len;
}

static void add_snowflake(void) {
	Snowflake snowflake;
	snowflake.pos.x = (rand() % (max_dimensions().x + 2));
	snowflake.pos.y = 0;
	snowflake.ch = g_snowflake_charmap[(int)(rand() % g_snowflake_charmap_len)].ch;
	g_snowflakes[snowflakes_len()] = snowflake;
}

static void update_snowflakes(void) {
	size_t i;
	for (i = 0; i < g_snowflakes_count; i++) {
		int max_height, new_height;
		Snowflake* snowflake;
		float v;
		if (g_snowflakes[i].ch == 0)
			break;
		snowflake = &g_snowflakes[i];
		max_height = max_dimensions().y;
		new_height = snowflake->pos.y;

		{
			/* lookup the snowflake char in the map */
			size_t j;
			v = 1.0f;
			for (j = 0; j < g_snowflake_charmap_len; j++) {
				if (g_snowflake_charmap[j].ch == snowflake->ch) {
					v = g_snowflake_charmap[j].v;
					break;
				}
			}
		}
		if (random_float() < v) {
			new_height += 1;
			if (new_height > max_height) {
				new_height -= 1;
			} else {
				/* check if a snowflake with pos (new_height, snowflake->pos.x) already exists */
				size_t k;
				for (k = 0; k < g_snowflakes_count; k++) {
					if (k == i)
						continue;
					if (g_snowflakes[k].pos.x == snowflake->pos.x && g_snowflakes[k].pos.y == new_height) {
						new_height -= 1;
					}
				}
			}
		}
		snowflake->pos.y = new_height;
	}
}

static void redisplay(void) {
	size_t i;
	for (i = 0; i < g_snowflakes_count; i++) {
		Snowflake* snowflake;
		Vec2 hw;
		if (g_snowflakes[i].ch == 0)
			break;
		hw = max_dimensions();
		snowflake = &g_snowflakes[i];
		if (snowflake->pos.y > hw.y || snowflake->pos.y >= hw.x)
			continue;
		mvaddch(snowflake->pos.y, snowflake->pos.x, snowflake->ch);
	}
}

static void draw_moon(void) {
	static char* moon[] = {
			/* clang-format off */
			"  **   ",
			"   *** ",
			"    ***",
			"    ***",
			"   *** ",
			"  **   ",
			NULL,
			/* clang-format on */
	};
	int start_position, i, height;
	char* line;

	height = 1;

	start_position = max_dimensions().x - 10;
	attrset(COLOR_PAIR(1));
	for (i = 0;; i++) {
		line = moon[i];
		if (line == NULL)
			break;
		mvaddstr(height, start_position, line);
		height++;
	}
	attrset(COLOR_PAIR(0));
}

int main(int argc, char** argv) {
	int speed;

	speed = 100;
	if (argc > 1) {
		char* end;
		speed = (int)strtol(argv[1], &end, 10);
		if (end == argv[1]) {
			puts("Usage:");
			puts("./c-snowterm [SPEED]");
			puts("SPEED is integer representing percents.");
			puts("\nc-snowterm is a rewrite of python-snowterm (https://github.com/valignatev/python-snowterm/) in C");
			return 1;
		}
	}

	{
		Vec2 hw;

		srand(time(NULL));

		initscr();
		if (can_change_color()) {
			start_color();
			init_color(COLOR_BLACK, 0, 0, 0);
			init_color(COLOR_WHITE, 1000, 1000, 1000);
			init_color(COLOR_YELLOW, 1000, 1000, 0);
		}
		init_pair(1, COLOR_YELLOW, 0);

		curs_set(0);
		nodelay(stdscr, true);

		hw = max_dimensions();
		g_snowflakes_size = hw.x * hw.y * sizeof(Snowflake);
		g_snowflakes_count = hw.x * hw.y;
		g_snowflakes = malloc(g_snowflakes_size);
		memset(g_snowflakes, 0, g_snowflakes_size);
		while (true) {
			hw = max_dimensions();

			if (snowflakes_len() >= (size_t)(0.95f * ((float)hw.x * (float)hw.y))) {
				memset(g_snowflakes, 0, g_snowflakes_size);
			}

			update_snowflakes();
			add_snowflake();

			clear();

			draw_moon();
			redisplay();

			refresh();

			{
				float ratio;
				ratio = speed / 100.0f;
				if (ratio == 0.0f) {
					sleep_seconds(0.2f);
				} else {
					sleep_seconds(0.2f / ratio);
				}
			}

			if (getch() == 'q')
				break;
		}
		endwin();
	}

	free(g_snowflakes);

	return 0;
}
