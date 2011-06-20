#include <gtk/gtk.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <math.h>

void pdfscreenshot_draw_preview (GtkWidget *widget, cairo_t *cr, gpointer window) {

    int draw_width = gtk_widget_get_allocated_width (widget);
    int draw_height = gtk_widget_get_allocated_height (widget);
    int win_width = gtk_widget_get_allocated_width (window);
    int win_height = gtk_widget_get_allocated_height (window);

    double scale = fmin(1, fmin (1.0*draw_width / win_width, 1.0*draw_height / win_height));

    cairo_scale(cr, scale, scale);

    cairo_translate(cr, (draw_width  - scale * win_width)/2,
                        (draw_height - scale * win_height)/2);

    gtk_widget_draw(window, cr);
}

void pdfscreenshot_draw_to_pdf (GtkWidget *widget, const gchar* filename) {
    cairo_surface_t *surface = 
            cairo_pdf_surface_create(filename,
                    1.0*gtk_widget_get_allocated_width (widget),
                    1.0*gtk_widget_get_allocated_height (widget));
    cairo_t *cr  = cairo_create(surface);
    gtk_widget_draw(widget, cr);
    cairo_destroy(cr);
    cairo_surface_show_page(surface);
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
}


void pdfscreenshot_draw_to_svg (GtkWidget *widget, const gchar* filename) {
    cairo_surface_t *surface = 
            cairo_svg_surface_create(filename,
                    1.0*gtk_widget_get_allocated_width (widget),
                    1.0*gtk_widget_get_allocated_height (widget));
    cairo_t *cr  = cairo_create(surface);
    gtk_widget_draw(widget, cr);
    cairo_destroy(cr);
    cairo_surface_show_page(surface);
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
}


void
pdfscreenshot_take_shot (GtkButton *button, gpointer our_window) {
    GList *toplevels = gtk_window_list_toplevels();

    for (GList *iter = toplevels; iter; iter = g_list_next(iter)) {
        GtkWindow *window = GTK_WINDOW(iter->data);
        const gchar *title = gtk_window_get_title(window);
        if (our_window != window && title != NULL) {
            // This seems to be the "other" window
            g_object_ref(window);

            GtkWidget *chooser = gtk_file_chooser_dialog_new (
                "Save PDF screenshot",
                our_window,
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);

            char *filename = g_strdup_printf("%s.pdf", g_get_application_name());
            gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), filename);
            g_free(filename);


            gtk_window_set_transient_for (GTK_WINDOW(chooser), GTK_WINDOW(our_window));
            gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (chooser), TRUE);

            GtkFileFilter *pdf_filter = gtk_file_filter_new ();
            gtk_file_filter_add_pattern (pdf_filter, "*.pdf");
            gtk_file_filter_set_name (pdf_filter,"PDF files (*.pdf)");
            gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), pdf_filter);
            //gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(chooser), pdf_filter);

            GtkFileFilter *svg_filter = gtk_file_filter_new ();
            gtk_file_filter_add_pattern (svg_filter, "*.svg");
            gtk_file_filter_set_name (svg_filter,"SVG files (*.svg)");
            gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), svg_filter);
            gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(chooser), svg_filter);

            GtkWidget *format_combo = gtk_combo_box_text_new ();
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(format_combo)
                                     , "pdf" ,"PDF Files (*.pdf)");
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(format_combo)
                                     , "svg" ,"SVG Files (*.svg)");
            gtk_combo_box_set_active(GTK_COMBO_BOX(format_combo),0);


            GtkWidget *drawing_area = gtk_drawing_area_new ();
            g_signal_connect (G_OBJECT (drawing_area), "draw",
                    G_CALLBACK (pdfscreenshot_draw_preview), window);
            gtk_widget_set_size_request (drawing_area, 300, 300);
        
            GtkWidget *frame = gtk_frame_new("Preview");
            gtk_container_add(GTK_CONTAINER(frame), drawing_area);

	    GtkWidget *vbox = gtk_vbox_new(FALSE, 8);
            gtk_box_pack_start(GTK_BOX(vbox), format_combo, FALSE, FALSE, 0);
            gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
            gtk_widget_show_all(vbox);

            gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(chooser), vbox);

            if (gtk_dialog_run (GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
                char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		//GtkFileFilter *the_filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(chooser));
                const char *the_id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(format_combo));
		if (!g_strcmp0(the_id,"pdf")) {
			pdfscreenshot_draw_to_pdf(GTK_WIDGET(window),filename);
		} else if (!g_strcmp0(the_id,"svg")) {
			pdfscreenshot_draw_to_svg(GTK_WIDGET(window),filename);
		} else {
                    printf("Unknown id \"%s\"\n", the_id);
                }
                g_free(filename);                        
            }

            gtk_widget_destroy (chooser);
            g_object_unref(window);
            g_list_free(toplevels);
            return;
        }
    }
}

void
pdfscreenshot_window_create()
{
    GtkWidget *button = gtk_button_new_with_label("Take screenshot...");

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"PDF Screenshot taker");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(window),TRUE);
    gtk_container_add(GTK_CONTAINER(window), button);

    g_signal_connect(G_OBJECT(button), "clicked",
        G_CALLBACK(pdfscreenshot_take_shot), window);


    gtk_widget_show_all(GTK_WIDGET(window));
}

int
gtk_module_init(gint argc, char *argv[])
{
    pdfscreenshot_window_create();
    return FALSE;
}


