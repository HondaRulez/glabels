/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  object-editor.c:  object properties editor module
 *
 *  Copyright (C) 2003  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include <config.h>

#include <gnome.h>
#include <math.h>

#include "object-editor.h"
#include "prefs.h"

#include "object-editor-private.h"

#include "debug.h"

/*===========================================*/
/* Private macros                            */
/*===========================================*/

#define LENGTH(x,y) sqrt( (x)*(x) + (y)*(y) )
#define ANGLE(x,y)  ( (180.0/G_PI)*atan2( -(y), (x) ) )
#define COMP_X(l,a) ( (l) * cos( (G_PI/180.0)*(a) ) )
#define COMP_Y(l,a) ( -(l) * sin( (G_PI/180.0)*(a) ) )
                                                                                

/*===========================================*/
/* Private data types                        */
/*===========================================*/

/*===========================================*/
/* Private globals                           */
/*===========================================*/

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/



/*--------------------------------------------------------------------------*/
/* PRIVATE.  Prepare line size page.                                        */
/*--------------------------------------------------------------------------*/
void
gl_object_editor_prepare_lsize_page (glObjectEditor       *editor)
{
	const gchar  *units_string;
	gdouble       climb_rate;
	gint          digits;
	GtkSizeGroup *label_size_group;
	GtkWidget    *label;

	gl_debug (DEBUG_EDITOR, "START");

	/* Extract widgets from XML tree. */
	editor->priv->lsize_page_vbox =
		glade_xml_get_widget (editor->priv->gui, "lsize_page_vbox");
	editor->priv->lsize_r_spin =
		glade_xml_get_widget (editor->priv->gui, "lsize_r_spin");
	editor->priv->lsize_theta_spin =
		glade_xml_get_widget (editor->priv->gui, "lsize_theta_spin");
	editor->priv->lsize_r_units_label =
		glade_xml_get_widget (editor->priv->gui, "lsize_r_units_label");

	/* Get configuration information */
	units_string = gl_prefs_get_units_string ();
	editor->priv->units_per_point = gl_prefs_get_units_per_point ();
	climb_rate = gl_prefs_get_units_step_size ();
	digits = gl_prefs_get_units_precision ();

	/* Modify widgets based on configuration */
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin), digits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON(editor->priv->lsize_r_spin),
					climb_rate, 10.0*climb_rate);
	gtk_label_set_text (GTK_LABEL(editor->priv->lsize_r_units_label), units_string);

	/* Align label widths */
	label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
	label = glade_xml_get_widget (editor->priv->gui, "lsize_r_label");
	gtk_size_group_add_widget (label_size_group, label);
	label = glade_xml_get_widget (editor->priv->gui, "lsize_theta_label");
	gtk_size_group_add_widget (label_size_group, label);

	/* Un-hide */
	gtk_widget_show_all (editor->priv->lsize_page_vbox);

	/* Connect signals */
	g_signal_connect_swapped (G_OBJECT (editor->priv->lsize_r_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));
	g_signal_connect_swapped (G_OBJECT (editor->priv->lsize_theta_spin),
				  "changed",
				  G_CALLBACK (gl_object_editor_changed_cb),
				  G_OBJECT (editor));

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set line size.                                                            */
/*****************************************************************************/
void
gl_object_editor_set_lsize (glObjectEditor      *editor,
			    gdouble              dx,
			    gdouble              dy)
{
	gdouble r, theta;

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->lsize_r_spin),
					 gl_object_editor_changed_cb,
					 editor);
	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->lsize_theta_spin),
					 gl_object_editor_changed_cb,
					 editor);

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal dx,dy = %g, %g", dx, dy);
	dx *= editor->priv->units_per_point;
	dy *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display dx,dy = %g, %g", dx, dy);

	r     = LENGTH (dx, dy);
	theta = ANGLE (dx, dy);

	/* Set widget values */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin), r);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_theta_spin),
				   theta);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->lsize_r_spin),
					   gl_object_editor_changed_cb,
					   editor);
	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->lsize_theta_spin),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Set maximum line size.                                                    */
/*****************************************************************************/
void
gl_object_editor_set_max_lsize (glObjectEditor      *editor,
				gdouble              dx_max,
				gdouble              dy_max)
{
	gdouble tmp;

	gl_debug (DEBUG_EDITOR, "START");

	g_signal_handlers_block_by_func (G_OBJECT(editor->priv->lsize_r_spin),
					 gl_object_editor_changed_cb,
					 editor);

	/* convert internal units to displayed units */
	gl_debug (DEBUG_EDITOR, "internal dx_max,dy_max = %g, %g", dx_max, dy_max);
	dx_max *= editor->priv->units_per_point;
	dy_max *= editor->priv->units_per_point;
	gl_debug (DEBUG_EDITOR, "display dx_max,dy_max = %g, %g", dx_max, dy_max);

	/* Set widget values */
	tmp = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin));
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin),
				   0.0, 2.0*LENGTH (dx_max, dy_max));
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin), tmp);

	g_signal_handlers_unblock_by_func (G_OBJECT(editor->priv->lsize_r_spin),
					   gl_object_editor_changed_cb,
					   editor);

	gl_debug (DEBUG_EDITOR, "END");
}

/*****************************************************************************/
/* Query line size.                                                          */
/*****************************************************************************/
void
gl_object_editor_get_lsize (glObjectEditor      *editor,
			    gdouble             *dx,
			    gdouble             *dy)
{
	gdouble r, theta;

	gl_debug (DEBUG_EDITOR, "START");

	/* Get values from widgets */
	r = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_r_spin));
	theta = gtk_spin_button_get_value (GTK_SPIN_BUTTON (editor->priv->lsize_theta_spin));

	/* convert everything back to our internal units (points) */
	r /= editor->priv->units_per_point;

	*dx = COMP_X (r, theta);
	*dy = COMP_Y (r, theta);

	gl_debug (DEBUG_EDITOR, "END");
}
