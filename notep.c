#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data);
static GtkWidget *create_menu_bar(void);
static void open_activate(void);
static void save_activate(void);
static void saveas_activate(void);
static void font_activate(void);

void
activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *menu_bar;
	GtkWidget *text_view;

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "notep");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid);

	menu_bar = create_menu_bar();
	gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);

	text_view = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
	gtk_widget_set_hexpand(text_view, TRUE);
	gtk_widget_set_vexpand(text_view, TRUE);
	gtk_grid_attach_next_to(GTK_GRID(grid), text_view, menu_bar,
	                        GTK_POS_BOTTOM, 1, 1);

	gtk_widget_show_all(window);
}

GtkWidget *
create_menu_bar(void)
{
	GtkWidget *menu_bar;
	GtkWidget *open;
	GtkWidget *save;
	GtkWidget *saveas;
	GtkWidget *font;

	menu_bar = gtk_menu_bar_new();

	open = gtk_menu_item_new_with_label("Open");
	g_signal_connect(G_OBJECT(open), "activate",
	                 G_CALLBACK(open_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), open);

	save = gtk_menu_item_new_with_label("Save");
	g_signal_connect(G_OBJECT(save), "activate",
	                 G_CALLBACK(save_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), save);

	saveas = gtk_menu_item_new_with_label("SaveAs");
	g_signal_connect(G_OBJECT(saveas), "activate",
	                 G_CALLBACK(saveas_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), saveas);

	font = gtk_menu_item_new_with_label("Font");
	g_signal_connect(G_OBJECT(font), "activate",
	                 G_CALLBACK(font_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), font);

	return menu_bar;
}

void
open_activate(void)
{
	g_print("open\n");
}

void
save_activate(void)
{
	g_print("save\n");
}

void
saveas_activate(void)
{
	g_print("saveas\n");
}

void
font_activate(void)
{
	g_print("font\n");
}

int
main(int argc, char *argv[])
{
	GtkApplication *app;
	int status;

	app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
