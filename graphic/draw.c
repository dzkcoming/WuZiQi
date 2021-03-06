#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>

void start_game(void);
void game_about(void);
static void image_play(GtkWidget *frame, const char *url, int size_x, int size_y, int x, int y);

typedef struct
{
	gint x;
	gint y;
}RECT_POINTER;

static const char *src_image[3] = {"resource/main_surface.png", "resource/about.jpg", "resource/game_background.jpg"};

GtkWidget *window, *frame;
GtkWidget *exit_button, *ack_button;
GtkWidget *image;

#define WINDOW_X 400
#define WINDOW_Y 400
#define LINE_SPACING  20
#define ROW_SPACING   20
#define CHESS_START_X 20
#define CHESS_START_Y 20
#define LINE_NUM ((WINDOW_Y - 2 * CHESS_START_Y) / LINE_SPACING + 1)
#define ROW_NUM  ((WINDOW_X - 2 * CHESS_START_X) / ROW_SPACING + 1)
#define IS_VALID_COOR(i, j) (i < ROW_NUM && j < LINE_NUM)

typedef enum {
	RIGHT      = 0x1,
	DOWN       = 0x2,
	RIGHT_DOWN = 0x4,
	RIGHT_UP   = 0x8,
}piece_direction;

char chess_map[LINE_NUM][ROW_NUM] = {{0}, };
char chess_black[LINE_NUM][ROW_NUM] = {{0}, };
char chess_white[LINE_NUM][ROW_NUM] = {{0}, };

piece_direction get_next_piece_direction(const char (*p)[ROW_NUM], int *get_row, int *get_line, int *len)
{
	piece_direction ret = RIGHT, ret1;
	piece_direction tmp;

	int i, j, i1, j1;
	int max_len, max_len1, cur_len;

	max_len = max_len1 = 0;
	for (i = 0; i < LINE_NUM; i++) {
		for (j = 0; j < ROW_NUM; j++) {
			max_len1 = max_len;
			if (p[i][j] != 0) {

				tmp = RIGHT;
				cur_len = 1;
				i1 = i; j1 = j;
				while (IS_VALID_COOR(i1, j1 + 1) && p[i1][j1 + 1] == 1) {
					cur_len++;
					j1++;
				}

				if (cur_len > max_len1) {
					max_len1 = cur_len;
					ret1 = tmp;
				}

				tmp = DOWN;
				cur_len = 1;
				i1 = i; j1 = j;
				while (IS_VALID_COOR(i1 + 1, j1) && p[i1 + 1][j1] == 1) {
					cur_len++;
					i1++;
				}

				if (cur_len > max_len1) {
					max_len1 = cur_len;
					ret1 = tmp;
				}


				tmp = RIGHT_DOWN;
				cur_len = 1;
				i1 = i; j1 = j;
				while (IS_VALID_COOR(i1 + 1, j1 + 1) && p[i1 + 1][j1 + 1] == 1) {
					cur_len++;
					i1++;
					j1++;
				}

				if (cur_len > max_len1) {
					max_len1 = cur_len;
					ret1 = tmp;
				}

				tmp = RIGHT_UP;
				cur_len = 1;
				i1 = i; j1 = j;
				while (IS_VALID_COOR(i1 - 1, j1 + 1) && p[i1 - 1][j1 + 1] == 1) {
					cur_len++;
					i1--;
					j1++;
				}

				if (cur_len > max_len1) {
					max_len1 = cur_len;
					ret1 = tmp;
				}

			}

			if (max_len1 > max_len) {
				max_len = max_len1;
				ret = ret1;
				*get_line = i;
				*get_row = j;
				*len = max_len;
			}
		}
	}

	return ret;
}

/*
 * x[out] : line
 * y[out] : row
 *
 */
int get_next_coordinate(const char (*base)[ROW_NUM], char (*change)[ROW_NUM], int *x, int *y)
{
	piece_direction dire;
	int co_line, co_row, len;
	int i, j;

	dire = get_next_piece_direction(base, &co_row, &co_line, &len);
	printf("=== dire %x line %d row %d =========\n", dire, co_line, co_row);

	/*  当定下一个方向后，会有两种填棋子的方法，例如，方向为右，坐标为 (1,1)，假设长度为 1
	 *  则会有两种选择，填在左边(0, 1)，填在右边(2, 1)，这里设置默认是先填的右边(2, 1)，这个后面改进
	 */
	// 以下的代码是基于 x 为横坐标， y 为纵坐标， 0,0 为坐标左上角
	switch (dire) {
	case RIGHT:

		i = co_line; j = co_row + len;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
				goto right_ok;

		i = co_line; j = co_row - 1;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
				goto right_ok;

		// 放在自己的最小行且同行中最小列的棋子的旁边
		for (i = 0; i < LINE_NUM; i++) {
			for (j = 0; j < ROW_NUM; j++) {
				if (change[i][j] == 1) {
					if (IS_VALID_COOR(i, j + 1) && chess_map[i][j + 1] == 0) {
						i = i; j = j + 1;
						goto right_ok;
					} else if (IS_VALID_COOR(i + 1, j) && chess_map[i + 1][j] == 0) {
						i = i + 1; j = j;
						goto right_ok;
					} else if (IS_VALID_COOR(i, j - 1) && chess_map[i][j -1] == 0) {
						i = i; j = j - 1;
						goto right_ok;
					} else if (IS_VALID_COOR(i - 1, j) && chess_map[i -1 ][j] == 0) {
						i = i - 1; j = j;
						goto right_ok;
					}
				}
			}
		}
right_ok:
		*x = i;
		*y = j;
		break;

	case DOWN:

		i = co_line + len; j = co_row;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
			goto down_ok;

		i = co_line - 1; j = co_row;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
			goto down_ok;

		for (i = 0; i < LINE_NUM; i++) {
			for (j = 0; j < ROW_NUM; j++) {
				if (change[i][j] == 1) {
					if (IS_VALID_COOR(i, j + 1) && chess_map[i][j + 1] == 0) {
						i = i; j = j + 1;
						goto down_ok;
					} else if (IS_VALID_COOR(i + 1, j) && chess_map[i + 1][j] == 0) {
						i = i + 1; j = j;
						goto down_ok;
					} else if (IS_VALID_COOR(i, j - 1) && chess_map[i][j -1] == 0) {
						i = i; j = j - 1;
						goto down_ok;
					} else if (IS_VALID_COOR(i - 1, j) && chess_map[i -1 ][j] == 0) {
						i = i - 1; j = j;
						goto down_ok;
					}
				}
			}
		}
down_ok:
		*x = i;
		*y = j;
		break;

	case RIGHT_DOWN:

		i = co_line + len; j = co_row + len;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
			goto right_down_ok;

		i = co_line - 1; j = co_row - 1;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
			goto right_down_ok;

		for (i = 0; i < LINE_NUM; i++) {
			for (j = 0; j < ROW_NUM; j++) {
				if (change[i][j] == 1) {
					if (IS_VALID_COOR(i, j + 1) && chess_map[i][j + 1] == 0) {
						i = i; j = j + 1;
						goto right_down_ok;
					} else if (IS_VALID_COOR(i + 1, j) && chess_map[i + 1][j] == 0) {
						i = i + 1; j = j;
						goto right_down_ok;
					} else if (IS_VALID_COOR(i, j - 1) && chess_map[i][j -1] == 0) {
						i = i; j = j - 1;
						goto right_down_ok;
					} else if (IS_VALID_COOR(i - 1, j) && chess_map[i -1 ][j] == 0) {
						i = i - 1; j = j;
						goto right_down_ok;
					}
				}
			}
		}

right_down_ok:
		*x = i;
		*y = j;
		break;

	case RIGHT_UP:

		i = co_line - len; j = co_row + len;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
			goto right_up_ok;

		i = co_line + 1; j = co_row - 1;
		if (IS_VALID_COOR(i, j) && chess_map[i][j] == 0)
			goto right_up_ok;

		for (i = 0; i < LINE_NUM; i++) {
			for (j = 0; j < ROW_NUM; j++) {
				if (change[i][j] == 1) {
					if (IS_VALID_COOR(i, j + 1) && chess_map[i][j + 1] == 0) {
						i = i; j = j + 1;
						goto right_up_ok;
					} else if (IS_VALID_COOR(i + 1, j) && chess_map[i + 1][j] == 0) {
						i = i + 1; j = j;
						goto right_up_ok;
					} else if (IS_VALID_COOR(i, j - 1) && chess_map[i][j -1] == 0) {
						i = i; j = j - 1;
						goto right_up_ok;
					} else if (IS_VALID_COOR(i - 1, j) && chess_map[i -1 ][j] == 0) {
						i = i - 1; j = j;
						goto right_up_ok;
					}
				}
			}
		}
right_up_ok:
		*x = i;
		*y = j;
		break;

	default:
		printf("error happed in %s %d\n", __func__, __LINE__);
		break;
	}

	printf("%s, get line %d row %d\n", __func__, *x, *y);

	return 0;
}

/*
 * return : 1 : win, 0 : not win
 */
int is_win(int is_black)
{
	char (*p)[ROW_NUM];
	int i, j;

	if (is_black)
		p = chess_black;
	else
		p = chess_white;

#if 0
	printf("    ");
	for (i = 0; i < ROW_NUM; i++)
		printf("%3d", i);
	printf("\n");

	for (i = 0; i < LINE_NUM; i++) {
		printf("%2d : ", i);
		for (j = 0; j < ROW_NUM; j++) {
			printf("%2d ", p[i][j]);
		}
		printf("\n");
	}
#endif

	piece_direction direct;

	for (i = 0; i < LINE_NUM; i++) {
		for (j = 0; j < ROW_NUM; j++) {
			if (p[i][j] != 0) {
				if (ROW_NUM - j >= 5) {
					direct |= RIGHT;
					if (p[i][j] && p[i][j + 1] && p[i][j + 2] && p[i][j + 3] && p[i][j + 4])
						goto win;
				}

				if (LINE_NUM - i >= 5) {
					direct |= DOWN;
					if (p[i][j] && p[i + 1][j] && p[i + 2][j] && p[i + 3][j] && p[i + 4][j])
						goto win;
				}

				if (i >= 4 && (direct & RIGHT)) {
					direct |= RIGHT_UP;
					if (p[i][j] && p[i - 1][j + 1] && p[i - 2][j + 2] && p[i - 3][j + 3] && p[i - 4][j + 4])
						goto win;
				}

				if ((direct & RIGHT) && (direct & DOWN)) {
					direct |= RIGHT_DOWN;
					if (p[i][j] && p[i + 1][j + 1] && p[i + 2][j + 2] && p[i + 3][j + 3] && p[i + 4][j + 4])
						goto win;
				}
			}
		}
	}

	return 0;
win:
	return 1;
}

void wuziqi(void)
{
	int argc = 0;
	char **argv = NULL;

	gtk_init(&argc, &argv); //初始化很多东西，暂时不清楚
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if (window == NULL) {
		printf("window new failed\n");
		return ;
	}
	gtk_widget_set_usize(window, WINDOW_X, WINDOW_Y);
	gtk_widget_set_uposition(window, 100, 100);
	gtk_window_set_title(GTK_WINDOW(window), "五子棋");

	frame = gtk_fixed_new();
	if (frame == NULL) {
		// free
		printf("fixed new failed\n");
		goto show;
	}
	gtk_container_add(GTK_CONTAINER(window), frame);

	/* 背景图片 */
	image_play(frame, src_image[0], WINDOW_X, WINDOW_Y, 0, 0);

	/* button */
	ack_button  = gtk_button_new_with_label("开始游戏");
	exit_button = gtk_button_new_with_label("  关于  ");
	if (exit_button == NULL || ack_button == NULL) {
		// free
		printf("exit_button new failed\n");
		goto show;
	}
	gtk_widget_set_size_request( ack_button, 150, 80);
	gtk_widget_set_size_request(exit_button, 150, 80);

	gtk_fixed_put(GTK_FIXED(frame),  ack_button, 130,  80);
	gtk_fixed_put(GTK_FIXED(frame), exit_button, 130, 180);

	g_signal_connect(     window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect( ack_button, "clicked", G_CALLBACK(start_game), NULL);
	g_signal_connect(exit_button, "clicked", G_CALLBACK(game_about), NULL);

show:
	gtk_widget_show_all(window);
	gtk_main();

	return ;
}

void draw_chessboard(GtkWidget *wd)
{
	GdkColor color;
	GdkGC *gc;
	GdkSegment seg[LINE_NUM + ROW_NUM];
	int i;

	gc = gdk_gc_new(wd->window);
	color.red   = 0x0000;
	color.green = 0x0000;
	color.blue  = 0xffff;
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_rectangle(wd->window, gc, TRUE, 0, 0, WINDOW_X, WINDOW_Y);


	color.red   = 0x0000;
	color.green = 0x0000;
	color.blue  = 0x0000;
	gdk_gc_set_rgb_fg_color(gc, &color);

	for (i = 0; i < LINE_NUM; i++) {
		seg[i].x1 = 20;
		seg[i].y1 = LINE_SPACING * (i + 1);
		seg[i].x2 = WINDOW_X - 20;
		seg[i].y2 = LINE_SPACING * (i + 1);
	}
	for (; i < LINE_NUM + ROW_NUM; i++) {
		seg[i].x1 = ROW_SPACING * ((i - LINE_NUM) + 1);
		seg[i].y1 = 20;
		seg[i].x2 = ROW_SPACING * ((i - LINE_NUM) + 1);
		seg[i].y2 = WINDOW_Y - 20;
	}

//	printf("============= line : %d row : %d ================\n", LINE_NUM, ROW_NUM);

	gdk_draw_segments(wd->window, gc, seg, LINE_NUM + ROW_NUM);
}

#define TO_ROW(x)  (x / 20 - 1)
#define TO_LINE(x) (x / 20 - 1)
static void draw_start(GtkWidget *draw,GdkEventButton *event,RECT_POINTER *rect)
{
	GdkColor color;
	static GdkGC *gc, *gc_black, *gc_white, *gc_red;
	static int inited;
#if 1
	static int is_black = 1;
#endif
	char *black_win = "Congratulation , black is win";
	char *white_win = "Congratulation , white is win";
	int this_x, this_y;
	int x, y;


	static GdkFont *font = NULL;
	if (!font)
		font = gdk_font_load ("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");

	if (inited == 0) {
		gc_black = gdk_gc_new(draw->window);
		color.red   = 0x0;
		color.green = 0x0;
		color.blue  = 0x0;
		gdk_gc_set_rgb_fg_color(gc_black, &color);

		gc_white = gdk_gc_new(draw->window);
		color.red   = 0xffff;
		color.green = 0xffff;
		color.blue  = 0xffff;
		gdk_gc_set_rgb_fg_color(gc_white, &color);

		gc_red = gdk_gc_new(draw->window);
		color.red   = 0xffff;
		color.green = 0x0000;
		color.blue  = 0x0000;
		gdk_gc_set_rgb_fg_color(gc_red, &color);
	}

#if 0
	if (is_black)
		gc = gc_black;
	else
		gc = gc_white;
#endif

	// 灵敏
	if (((int)event->x % 20) <= 10 && ((int)event->y % 20) <= 10)
		;
	else
		return ;

	if ((event->x - 10 > 0) && (event->y - 10 > 0)) {
		x = (int)event->x / 20 * 20;
		y = (int)event->y / 20 * 20;
	} else
		x = y = 0;

	if (chess_map[TO_LINE(y)][TO_ROW(x)] == 1)
		return ;
	else
		chess_map[TO_LINE(y)][TO_ROW(x)] = 1;

	printf("================ line %d row %d black %d===================\n", TO_LINE(y), TO_ROW(x), is_black);

	gdk_draw_arc(draw->window, gc_black, TRUE, x - 10, y - 10, 20, 20, 0, 360 * 64);
	chess_black[TO_LINE(y)][TO_ROW(x)] = 1;
	if (is_win(1))
		gdk_draw_text(draw->window, font, gc_red, 80, 160, black_win, strlen(black_win));

	get_next_coordinate(chess_black, chess_white, &this_x, &this_y);
	this_x = (this_x + 1) * 20;
	this_y = (this_y + 1) * 20;
	gdk_draw_arc(draw->window, gc_white, TRUE, this_y - 10, this_x - 10, 20, 20, 0, 360 * 64);
	chess_map[TO_LINE(this_x)][TO_ROW(this_y)] = 1;
	chess_white[TO_LINE(this_x)][TO_ROW(this_y)] = 1;
	if (is_win(0))
		gdk_draw_text(draw->window, font, gc_red, 80, 160, white_win, strlen(black_win));

#if 0
	if (is_black) {
		chess_black[TO_LINE(y)][TO_ROW(x)] = 1;
		if (is_win(1)) {
			gdk_draw_text(draw->window, font, gc_red, 80, 160, black_win, strlen(black_win));
		}
	} else {
		chess_white[TO_LINE(y)][TO_ROW(x)] = 1;
		if (is_win(0)) {
			gdk_draw_text(draw->window, font, gc_red, 80, 160, white_win, strlen(white_win));
		}
	}

	is_black = !is_black;
#endif
}

static gboolean my_expose(GtkWidget      *da,
		GdkEventExpose *event,
		gpointer        data )
{
	RECT_POINTER rect;

	draw_chessboard(da);
	gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(da), "button_press_event", G_CALLBACK(draw_start), &rect);

	gtk_widget_show_all(da);
//	return FALSE;
}

void start_game(void)
{
	gtk_container_remove(GTK_CONTAINER(window), frame);

	GtkWidget *da;

	da = gtk_drawing_area_new ();
	gtk_container_add(GTK_CONTAINER(window), da);
    g_signal_connect(da, "expose_event", G_CALLBACK(my_expose), NULL );

	gtk_widget_show_all(window);
}


static const char *about_title = "关   于";
static const char *about_content = "作者: 窦中康\n\n源码地址: https://github.com/dzkcoming/Little_Game\n\n加入我们: 来吧，加入我们的队伍";

void game_about(void)
{
	gtk_container_remove(GTK_CONTAINER(window), frame);

	GtkWidget *fixed;
	GtkWidget *label_title, *label_content;
	fixed = gtk_fixed_new();
	if (fixed == NULL) {
		printf("fixed new failed\n");
	}
	gtk_container_add(GTK_CONTAINER(window), fixed);

	image_play(fixed, src_image[1], WINDOW_X, WINDOW_Y, 0, 0);

	label_title   = gtk_label_new(about_title);
	label_content = gtk_label_new(about_content);
	gtk_fixed_put(GTK_FIXED(fixed),   label_title,  180, 0);
	gtk_fixed_put(GTK_FIXED(fixed), label_content, 0, 80);


	gtk_widget_show_all(window);
}

static void image_play(GtkWidget *frame, const char *url, int size_x, int size_y, int x, int y)
{
	//5.通过GdkPixbuf来写该图片大小
	//5.1创建pixbuf，需要占用资源，使用完需要释放
	GdkPixbuf *src = gdk_pixbuf_new_from_file(url, NULL);
	//5.2修改图片大小
	GdkPixbuf *dst = gdk_pixbuf_scale_simple(src, size_x, size_y, GDK_INTERP_BILINEAR);
	GtkWidget *image = gtk_image_new_from_pixbuf(dst);
	//5.3pixbuf使用完需要释放其中资源
	g_object_unref(src);
	g_object_unref(dst);

	gtk_fixed_put(GTK_FIXED(frame), image, 0, 0);
}
