/* See LICENSE file for copyright and license details. */
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

/* Types */
enum {YES, NO, CANCEL}; /* unsaved_dialog */

/* Function declarations */
static void activate(GtkApplication *app, gpointer user_data);
static GtkWidget *create_menu_bar(void);
static GtkWidget *create_text_view(void);
static GtkWidget *create_font_chooser(void);

static void open_activate(void);
static int save_activate(void);
static int saveas_activate(void);
static void font_activate(void);
static void select_font(void);

static void load_file(void);
static int save_file(void);
static int unsaved_dialog(void);
static gboolean exit_notep(void);
static void die(char *msg);

/* Variables */
static GtkWidget *window;
static GtkWidget *text_view;
static GtkWidget *text_window;
static GtkWidget *font_chooser;
static GtkTextBuffer *buffer;
static PangoFontDescription *font;
static GtkBuilder *builder;
static int saved = 0;
static char *filename = NULL;

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
	g_signal_connect(window, "delete-event", G_CALLBACK(exit_notep), NULL);

	grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid);

	menu_bar = create_menu_bar();
	gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);

	text_window = create_text_view(); /* TODO create_text_window ? */
	gtk_grid_attach_next_to(GTK_GRID(grid), text_window, menu_bar,
	                        GTK_POS_BOTTOM, 1, 1);

	/* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view)); */

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
create_text_view(void)
{
	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);

	text_view = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
	gtk_widget_override_font(text_view, font);
	gtk_widget_set_hexpand(text_view, TRUE);
	gtk_widget_set_vexpand(text_view, TRUE);

	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	/* TODO buffer assignment should be general. Get it out of here */
	return scrolled_window;
}

GtkWidget *
create_font_chooser(void)
{
	GObject *select;
	GObject *cancel;

	font_chooser = gtk_font_chooser_dialog_new(NULL, GTK_WINDOW(window));

	gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(font_chooser), font);

	return font_chooser;
}

void
open_activate(void)
{
	GtkWidget *file_chooser;
	gint res;
	int prompt;

	if (gtk_text_buffer_get_modified(buffer) == TRUE) {
		prompt = unsaved_dialog();
		if (prompt ==  YES)
			prompt = save_activate();
		if (prompt == CANCEL)
			return;
		/* else 'button no' continues as normal */
	}

	file_chooser = gtk_file_chooser_dialog_new("Open file",
	                                           GTK_WINDOW(window),
	                                           GTK_FILE_CHOOSER_ACTION_OPEN,
	                                           "_Cancel",
	                                           GTK_RESPONSE_CANCEL,
	                                           "_Open",
	                                           GTK_RESPONSE_ACCEPT,
	                                           NULL);
	res = gtk_dialog_run(GTK_DIALOG(file_chooser));

	if (res == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(
			GTK_FILE_CHOOSER(file_chooser));
		load_file();
	}

	gtk_widget_destroy(file_chooser);
}

int
save_activate(void)
{
	int res = NO;

	if ((gtk_text_buffer_get_modified(buffer) == TRUE) || !saved)
		res = saved ? save_file() : saveas_activate();

	return res; /* TODO verify correctness of this return */
}

int
saveas_activate(void)
{
	GtkWidget *dialog;
	GtkFileChooser *chooser;
	gint res;

	dialog = gtk_file_chooser_dialog_new("Save file",
	                                     GTK_WINDOW(window),
	                                     GTK_FILE_CHOOSER_ACTION_SAVE,
	                                     "_Cancel",
	                                     GTK_RESPONSE_CANCEL,
	                                     "_Save",
	                                     GTK_RESPONSE_ACCEPT,
	                                     NULL);

	chooser = GTK_FILE_CHOOSER(dialog);
	gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

	if (filename)
		gtk_file_chooser_set_filename(chooser, filename);
		/* TODO is this nescessary? */

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		/* TODO g_free filename? */
		filename = gtk_file_chooser_get_filename(chooser);
		save_file();
		res = YES;
	} else {
		res = CANCEL;
	}

	gtk_widget_destroy(dialog);
	return res;
}

void
font_activate(void)
{
	int res;

	font_chooser = create_font_chooser();
	res = gtk_dialog_run(GTK_DIALOG(font_chooser));
	if (res == GTK_RESPONSE_OK)
		select_font();

	gtk_widget_destroy(font_chooser);
}

void
select_font(void)
{
	pango_font_description_free(font);
	font = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(font_chooser));
	gtk_widget_override_font(text_view, font);
	/* TODO text_window.get_children() */
}

void
load_file(void)
{
	FILE *file;
	size_t length;
	char *text;

	file = fopen(filename, "r");
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	text = malloc((size_t)(length + 1)); /* TODO error checking */
	if (fread(text, sizeof(char), length, file) != length)
		die("notep: error reading file");
	text[length] = '\0';
	gtk_text_buffer_set_text(buffer, text, length);

	saved = 1;
	gtk_text_buffer_set_modified(buffer, FALSE);
	fclose(file);
}

int
save_file(void)
{
	FILE *file;
	size_t length;
	char *text;
	GtkTextIter *start;
	GtkTextIter *end;

	start = malloc(sizeof(GtkTextIter));
	end = malloc(sizeof(GtkTextIter));

	file = fopen(filename, "w");
	length = (size_t)gtk_text_buffer_get_char_count(buffer);
	gtk_text_buffer_get_start_iter(buffer, start);
	gtk_text_buffer_get_end_iter(buffer, end);
	text = gtk_text_buffer_get_text(buffer, start, end, TRUE);
	fprintf(file, "%s", text);

	saved = 1;
	gtk_text_buffer_set_modified(buffer, FALSE);
	fclose(file);
	return YES;
}

int
unsaved_dialog(void)
{
	GtkWidget *dialog;
	gint res;

	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
	                                GTK_DIALOG_MODAL,
	                                GTK_MESSAGE_QUESTION,
	                                GTK_BUTTONS_NONE,
	                                "The document has been modified.\n"
	                                "Save changes?");
	gtk_dialog_add_buttons(GTK_DIALOG(dialog),
	                       "Yes", YES,
	                       "No", NO,
	                       "Cancel", CANCEL, NULL);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return res;
}

gboolean
exit_notep(void)
{
	int prompt;

	if (gtk_text_buffer_get_modified(buffer) == TRUE) {
		prompt = unsaved_dialog();
		if (prompt ==  YES)
			prompt = save_activate();
		if (prompt == CANCEL)
			return TRUE;
	}

	exit(EXIT_SUCCESS);
}

void
die(char *msg)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
	                                GTK_DIALOG_DESTROY_WITH_PARENT,
	                                GTK_MESSAGE_ERROR,
	                                GTK_BUTTONS_OK,
	                                "%s", msg);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	GtkApplication *app;
	int status;

	app = gtk_application_new("byllgrim.notep", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
