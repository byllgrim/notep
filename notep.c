#include <gtk/gtk.h>

static void
activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *menu_item;
	GtkWidget *menu_bar;
	GtkWidget *text_view;

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "notep");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid);

	menu_item = gtk_menu_item_new_with_label("hey");

	menu_bar = gtk_menu_bar_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
	gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);

	text_view = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
	gtk_widget_set_hexpand(text_view, TRUE);
	gtk_widget_set_vexpand(text_view, TRUE);
	gtk_grid_attach_next_to(GTK_GRID(grid), text_view, menu_bar,
	                        GTK_POS_BOTTOM, 1, 1);

	gtk_widget_show_all(window);
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
