#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

/* Function declarations */
static void activate(GtkApplication *app, gpointer user_data);

static GtkWidget *create_menu_bar(void);
static GtkWidget *create_font_chooser(void);

static void open_activate(void);
static void save_activate(void);
static void saveas_activate(void);
static void font_activate(void);
static void select_activate(GtkWidget *w);
static void cancel_activate(GtkWidget *w);

static void load_file(char *filename);

/* Variables */
static GtkWidget *window;
static GtkWidget *text_view;
static GtkWidget *font_chooser;
static PangoFontDescription *font;
static GtkBuilder *builder;

/* Function definitions */
void
activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *grid;
	GtkWidget *menu_bar;

	builder = gtk_builder_new();
	font = pango_font_description_from_string(default_font);

	window = gtk_application_window_new(app);
	/* TODO GDK_KEY_PRESS_MASK and key-press-event */
	gtk_window_set_title(GTK_WINDOW(window), "notep");
	gtk_window_set_default_size(GTK_WINDOW(window), 320, 240);

	grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid);

	menu_bar = create_menu_bar();
	gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);

	text_view = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
	gtk_widget_override_font(text_view, font);
	pango_font_description_free(font);
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
	g_signal_connect(G_OBJECT(open), "button-press-event",
	                 G_CALLBACK(open_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), open);

	save = gtk_menu_item_new_with_label("Save");
	g_signal_connect(G_OBJECT(save), "button-press-event",
	                 G_CALLBACK(save_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), save);

	saveas = gtk_menu_item_new_with_label("SaveAs");
	g_signal_connect(G_OBJECT(saveas), "button-press-event",
	                 G_CALLBACK(saveas_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), saveas);

	font = gtk_menu_item_new_with_label("Font");
	g_signal_connect(G_OBJECT(font), "button-press-event",
	                 G_CALLBACK(font_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), font);

	return menu_bar;
}

GtkWidget *
create_font_chooser(void)
{
	GObject *select;
	GObject *cancel;

	font_chooser = gtk_font_chooser_dialog_new(NULL, GTK_WINDOW(window));

        select = gtk_buildable_get_internal_child(GTK_BUILDABLE(font_chooser),
                                                  builder, "select_button");
        cancel = gtk_buildable_get_internal_child(GTK_BUILDABLE(font_chooser),
                                                  builder, "cancel_button");

        g_signal_connect(select, "button-press-event",
                         G_CALLBACK(select_activate), NULL);
        g_signal_connect(cancel, "button-press-event",
                         G_CALLBACK(cancel_activate), NULL);

	/* TODO gtk_font_chooser_set_font(font_chooser, font); */

	return font_chooser;
}

void
open_activate(void)
{
	GtkWidget *file_chooser;
	gint res;

	file_chooser = gtk_file_chooser_dialog_new("Open file", window,
	                                           GTK_FILE_CHOOSER_ACTION_OPEN,
	                                           "_Cancel",
	                                           GTK_RESPONSE_CANCEL,
	                                           "_Open",
	                                           GTK_RESPONSE_ACCEPT,
	                                           NULL);
	res = gtk_dialog_run(file_chooser);

	if (res == GTK_RESPONSE_ACCEPT)
		load_file(gtk_file_chooser_get_filename(
			GTK_FILE_CHOOSER(file_chooser)));

	gtk_widget_destroy(file_chooser);
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
	GtkTextBuffer *buffer;

	font_chooser = create_font_chooser();
	gtk_dialog_run(GTK_DIALOG(font_chooser));
	/* TODO GTK_RESPONSE_CANCEL */

	/* TODO unref font_chooser? */
}

void
select_activate(GtkWidget *w)
{
	font = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(font_chooser));
	gtk_widget_override_font(text_view, font);
	pango_font_description_free(font);
	cancel_activate(w);
}

void
cancel_activate(GtkWidget *w)
{
	GtkWidget *parent = gtk_widget_get_parent(w);
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);

	/* GtkWindow *parent = GTK_WINDOW(gtk_widget_get_parent_window(w));
	 * TODO can it be done prettier?
	 */

	g_print("cancel\n");
	gtk_window_close(GTK_WINDOW(parent));
}

void
load_file(char *filename)
{
	FILE *file;
	long length;
	char *text;
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	file = fopen(filename, "r");
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	text = malloc((size_t)(length + 1)); /* TODO error checking */
	fread(text, sizeof(char), length, file);
	text[length] = '\0';
	gtk_text_buffer_set_text(buffer, text, length);
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

	/* TODO delete event to prevent unsaved exit */
	return status;
}
