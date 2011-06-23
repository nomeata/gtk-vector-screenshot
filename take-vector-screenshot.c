/*
 * © 2011 Joachim Breitner <mail@joachim-breitner.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

GdkAtom pdfscreenshot_atom;
char *supported_str = "supported";

GtkWidget *grab_window;
GtkWidget *button;

Window selected_window = None;

gboolean
pdfscreenshot_window_selected(GtkWidget *grab_window,
                  GdkEventButton *event,
                  gpointer *userdata)
{
    gdk_pointer_ungrab(event->time);

    int dummy;
    unsigned int dummyU;
    Window dummyW;

    XQueryPointer(gdk_x11_get_default_xdisplay(),
        gdk_x11_get_default_root_xwindow(),
        &dummyW, &selected_window, &dummy, &dummy, &dummy, &dummy, &dummyU);

    XClientMessageEvent xevent;
    xevent.type = ClientMessage;
    xevent.message_type = gdk_x11_atom_to_xatom(pdfscreenshot_atom);
    xevent.format = 32;
    xevent.window  = selected_window;
    XSendEvent(gdk_x11_get_default_xdisplay(), selected_window, 0, 0, (XEvent *)&xevent);

    gtk_button_released(GTK_BUTTON(button));

    return FALSE;
}


gboolean
pdfscreenshot_select_window(GtkWidget *button, GdkEvent *event, gpointer userdata)
{
    GdkCursor *cursor;

    GdkEventMask events = GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;

    cursor = gdk_cursor_new_for_display(gtk_widget_get_display(button),
                                        GDK_CROSSHAIR);
    gdk_device_grab(
        gdk_event_get_device(event),
        gtk_widget_get_window(grab_window),
        GDK_OWNERSHIP_APPLICATION,
        FALSE,
        events,
        cursor,
        event->button.time);

    gdk_cursor_unref(cursor);

    return TRUE;
}


/*
 * The main window, with the one button.
 */
void
pdfscreenshot_window_create()
{
    // Is that icon always installed or do we have to ship it?
    GtkWidget *icon = gtk_image_new_from_icon_name ("camera",GTK_ICON_SIZE_BUTTON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 128);

    button = gtk_button_new_with_label("Take vector screenshot...");
    gtk_button_set_image (GTK_BUTTON(button), icon);
    gtk_button_set_image_position (GTK_BUTTON(button), GTK_POS_TOP);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"Vector screenshot taker");
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(window),TRUE);
    gtk_container_add(GTK_CONTAINER(window), button);

    g_signal_connect(G_OBJECT(button), "button_release_event",
        G_CALLBACK(pdfscreenshot_select_window), NULL);

    gtk_widget_show_all(GTK_WIDGET(window));
}

void
pdfscreenshot_grab_window_create()
{
    GdkEventMask events = GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK;

    grab_window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_widget_show(grab_window);
    gtk_window_resize(GTK_WINDOW(grab_window), 1, 1);
    gtk_window_move(GTK_WINDOW(grab_window), -100, -100);
    gtk_widget_add_events(grab_window, events);
    g_signal_connect(G_OBJECT(grab_window), "button_release_event",
        G_CALLBACK(pdfscreenshot_window_selected), NULL);
}

int
main(gint argc, char *argv[])
{
    gtk_init(&argc, &argv);

    pdfscreenshot_atom = gdk_atom_intern ("GTK_VECTOR_SCREENSHOT", FALSE);
    pdfscreenshot_window_create();
    pdfscreenshot_grab_window_create();

    gtk_main();

    return 0;
}

