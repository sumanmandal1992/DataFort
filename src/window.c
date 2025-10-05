#include "window.h"
#include "dataquery.h"
#include "glibconfig.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct DataFort DataFort;
static DataFort *df = NULL;
static const gchar **company = NULL, **product = NULL;
static void createWidgets();
static void paneConfig();
static void stackConfig();
static void sale_entry();
static void sale_warn_comp(GtkWidget *widget, gpointer data);
static void sale_warn_prod(GtkWidget *widget, gpointer data);
static void save_sale_data(GtkWidget *widget, gpointer data);
static void sale_bill_entry();
static void purchase_entry();
static void save_purchase_data(GtkWidget *widget, gpointer data);
static void purchase_bill_entry();
static void query_database();
static void query_result(GtkWidget *, gpointer);
static void login_database();
static void update_database(GtkWidget *, gpointer);
static void company_product_list();
static void logout_db(GtkWidget *, gpointer);
static void update_selected_db(GtkWidget *, gpointer);
static void free_memory(GtkWidget *, gpointer);

/**********************
 * Widgets structure. *
 **********************/
struct DataFort {
  // Pane
  GtkWidget *hpane;
  GtkWidget *leftFrame;
  GtkWidget *rightFrame;

  GtkWidget *hpanedSale;
  GtkWidget *hpanedSaleBill;
  GtkWidget *hpanedPurchase;
  GtkWidget *hpanedPurchaseBill;
  GtkWidget *hpanedQuery;
  GtkWidget *hpanedQueryResult;
  GtkWidget *hpanedUpdateDb;

  // Stack
  GtkWidget *stackMain;
  GtkWidget *stackSidebar;
};

/*******************
 * Create widgets. *
 *******************/
static void createWidgets() {
  // Panes
  df->hpane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  df->leftFrame = gtk_frame_new(NULL);
  df->rightFrame = gtk_frame_new(NULL);

  df->hpanedSale = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  df->hpanedSaleBill = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  df->hpanedPurchase = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  df->hpanedPurchaseBill = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  df->hpanedQuery = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  df->hpanedQueryResult = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
  df->hpanedUpdateDb = gtk_paned_new(GTK_ORIENTATION_VERTICAL);

  // Stacks
  df->stackMain = gtk_stack_new();
  df->stackSidebar = gtk_stack_sidebar_new();
}

/************************
 * Pane Configurations. *
 ************************/
static void paneConfig() {
  // Top Pane
  gtk_widget_set_size_request(df->hpane, 760, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpane), df->leftFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpane), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpane), FALSE);
  gtk_widget_set_size_request(df->leftFrame, 150, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpane), df->rightFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpane), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpane), FALSE);
  gtk_widget_set_size_request(df->rightFrame, 610, -1);

  gtk_frame_set_child(GTK_FRAME(df->leftFrame), df->stackSidebar);
  gtk_frame_set_child(GTK_FRAME(df->rightFrame), df->stackMain);
}

/*************************
 * Stack Configurations. *
 *************************/
static void stackConfig() {
  // Main Stack
  const gchar *StackItems[] = {"Sale Entry",     "Sale Bill Entry",
                               "Purchase Entry", "Purchase Bill Entry",
                               "Database Query", "Update db (admin)"};
  gtk_stack_add_titled(GTK_STACK(df->stackMain), df->hpanedSale, "hpanedSale", StackItems[0]);
  gtk_stack_add_titled(GTK_STACK(df->stackMain), df->hpanedSaleBill, "hpanedSaleBill", StackItems[1]);
  gtk_stack_add_titled(GTK_STACK(df->stackMain), df->hpanedPurchase, "hpanedPurchase", StackItems[2]);
  gtk_stack_add_titled(GTK_STACK(df->stackMain), df->hpanedPurchaseBill, "hpanedPurchaseBill", StackItems[3]);
  gtk_stack_add_titled(GTK_STACK(df->stackMain), df->hpanedQuery, "hpanedQuery", StackItems[4]);
  gtk_stack_add_titled(GTK_STACK(df->stackMain), df->hpanedUpdateDb, "hpanedUpdateDb", StackItems[5]);
  gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(df->stackSidebar), GTK_STACK(df->stackMain));
}

static void company_product_list() {
  // Quering data from database...
  const char *querygst = "select gstin FROM companies";
  const char *querypid = "select pid FROM products";
  QueryRes *resgst = dbquery(querygst, Company);
  QueryRes *respid = dbquery(querypid, Company);

  unsigned long gst = 2;
  unsigned long pd = 2;

  if(resgst != NULL) gst = resgst->num_rows + 2;
  if(resgst != NULL) pd = respid->num_rows + 2;

  const gchar **gstin = (const gchar **)malloc(sizeof(gchar *) * gst);
  const gchar **pid = (const gchar **)malloc(sizeof(gchar *) * pd);
  gstin[0] = "Select Company...";
  pid[0] = "Select Product...";

  unsigned long i = 0;
  MYSQL_ROW row = NULL;

  if (resgst != NULL) {
    i = 1;
    while ((row = mysql_fetch_row(resgst->result))) {
      gstin[i++] = row[0];
    }
    gstin[i] = NULL;
  } else gstin[1] = NULL;

  if(respid != NULL) {
    i = 1;
    while ((row = mysql_fetch_row(respid->result))) {
      pid[i++] = row[0];
    }
    pid[i] = NULL;
  } else pid[1] = NULL;

  company = gstin;
  product = pid;
  free(resgst);
  free(respid);
}

/**************
 * Sale Entry *
 * ************/
typedef struct _SaleWarn {
  GtkWidget *compwarn;
  GtkWidget *prodwarn;
} SaleWarn;

static SaleWarn warn;
static void sale_entry() {
  // Create widgets
  GtkWidget *topFrame = gtk_frame_new(NULL);
  GtkWidget *bottomFrame = gtk_frame_new(NULL);
  GtkWidget *topLabel = gtk_label_new("Sale Entry");
  GtkWidget *grid = gtk_grid_new();
  warn.compwarn = gtk_label_new("*Select Company");
  warn.prodwarn = gtk_label_new("*Select Product");
  unsigned int lb = 6;
  GtkWidget *label[lb];
  GtkWidget *entry[lb];
  GtkWidget *savebtn = gtk_button_new_with_label("Save");
  const gchar *const lbs[] = {"Company",      "Product ID", "Date",
                              "Product Name", "Quantity",   "Price / Quantity"};
  const gchar *const days[] = {
      "dd", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10",
      "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21",
      "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", NULL};
  const gchar *const months[] = {"mm", "01", "02", "03", "04", "05", "06",
                                 "07", "08", "09", "10", "11", "12", NULL};
  const gchar *const years[] = {"yyyy", "2024", "2025", "2026", "2027",
                                "2028", "2029", "2030", NULL};

  for (unsigned int i = 0; i < lb; i++) {
    label[i] = gtk_label_new(lbs[i]);
  }

  entry[0] = gtk_drop_down_new_from_strings(company);
  entry[1] = gtk_drop_down_new_from_strings(product);
  entry[2] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  GtkWidget *day = gtk_drop_down_new_from_strings(days);
  GtkWidget *month = gtk_drop_down_new_from_strings(months);
  GtkWidget *year = gtk_drop_down_new_from_strings(years);
  gtk_box_append(GTK_BOX(entry[2]), day);
  gtk_box_append(GTK_BOX(entry[2]), month);
  gtk_box_append(GTK_BOX(entry[2]), year);
  gtk_drop_down_set_show_arrow(GTK_DROP_DOWN(day), FALSE);
  gtk_drop_down_set_show_arrow(GTK_DROP_DOWN(month), FALSE);
  gtk_drop_down_set_show_arrow(GTK_DROP_DOWN(year), FALSE);

  for (unsigned int j = 3; j < lb; j++)
    entry[j] = gtk_entry_new();

  // Pane Config
  gtk_widget_set_size_request(df->hpanedSale, 200, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpanedSale), topFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpanedSale), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpanedSale), FALSE);
  gtk_widget_set_size_request(topFrame, 50, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpanedSale), bottomFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpanedSale), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpanedSale), FALSE);
  gtk_widget_set_size_request(bottomFrame, 150, -1);

  gtk_frame_set_child(GTK_FRAME(topFrame), topLabel);
  gtk_frame_set_child(GTK_FRAME(bottomFrame), grid);

  // Signal
  g_signal_connect_after(entry[0], "notify::selected-item",
                         G_CALLBACK(sale_warn_comp), NULL);
  g_signal_connect_after(entry[1], "notify::selected-item",
                         G_CALLBACK(sale_warn_prod), NULL);
  g_signal_connect(savebtn, "clicked", G_CALLBACK(save_sale_data), grid);

  // Grid Config
  //
  for (unsigned int i = 0; i < lb; i++) {
    gtk_grid_attach(GTK_GRID(grid), label[i], 0, i, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry[i], 1, i, 1, 1);
  }
  gtk_grid_attach(GTK_GRID(grid), warn.compwarn, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), warn.prodwarn, 2, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), savebtn, 0, lb, 1, 1);

  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
  gtk_widget_set_margin_start(grid, 10);
  gtk_widget_set_margin_top(grid, 5);
  for (unsigned int j = 0; j < lb; j++)
    gtk_widget_set_halign(label[j], GTK_ALIGN_START);
}

/****************
 * Sale Warning *
 ****************/
static void sale_warn_comp(GtkWidget *widget, gpointer data) {
  (void)data;
  const guint dropp = gtk_drop_down_get_selected(GTK_DROP_DOWN(widget));
  gtk_widget_set_visible(warn.compwarn, TRUE);

  if (dropp > 0) {
    gtk_widget_set_visible(warn.compwarn, FALSE);
  }
}

static void sale_warn_prod(GtkWidget *widget, gpointer data) {
  (void)data;
  const guint dropp = gtk_drop_down_get_selected(GTK_DROP_DOWN(widget));
  gtk_widget_set_visible(warn.prodwarn, TRUE);

  if (dropp > 0) {
    gtk_widget_set_visible(warn.prodwarn, FALSE);
  }
}

/******************
 * Save Sale Data *
 ******************/
static void save_sale_data(GtkWidget *widget, gpointer data) {
  (void)widget;
  GtkWidget *compobj = gtk_grid_get_child_at(GTK_GRID(data), 1, 0);
  GtkWidget *prodobj = gtk_grid_get_child_at(GTK_GRID(data), 1, 1);

  const char *comp = gtk_string_object_get_string(GTK_STRING_OBJECT(
      gtk_drop_down_get_selected_item(GTK_DROP_DOWN(compobj))));
  const guint compp = gtk_drop_down_get_selected(GTK_DROP_DOWN(compobj));

  const char *prod = gtk_string_object_get_string(GTK_STRING_OBJECT(
      gtk_drop_down_get_selected_item(GTK_DROP_DOWN(prodobj))));
  const guint prodp = gtk_drop_down_get_selected(GTK_DROP_DOWN(prodobj));

  if (compp > 0 && prodp > 0) {
    printf("%s %s\n", comp, prod);
  }
}

/*******************
 * Sale Bill Entry *
 * *****************/
static void sale_bill_entry() {
  GtkWidget *topFrame = gtk_frame_new(NULL);
  GtkWidget *bottomFrame = gtk_frame_new(NULL);
  GtkWidget *topLabel = gtk_label_new("Sale Bill Entry");
  GtkWidget *grid = gtk_grid_new();
  unsigned int lb = 6;
  GtkWidget *label[lb];
  GtkWidget *entry[lb];
  GtkWidget *savebtn = gtk_button_new_with_label("Save");
  const gchar *const lbs[] = {"Company",      "Product ID", "Date",
                              "Product Name", "Quantity",   "Price / Quantity"};

  for (unsigned int i = 0; i < lb; i++) {
    label[i] = gtk_label_new(lbs[i]);
  }

  entry[0] = gtk_drop_down_new_from_strings(company);
  entry[1] = gtk_drop_down_new_from_strings(product);
  for (unsigned int j = 2; j < lb; j++)
    entry[j] = gtk_entry_new();

  // Sale Bill Entry Pane
  gtk_widget_set_size_request(df->hpanedSaleBill, 200, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpanedSaleBill), topFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpanedSaleBill), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpanedSaleBill), FALSE);
  gtk_widget_set_size_request(topFrame, 50, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpanedSaleBill), bottomFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpanedSaleBill), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpanedSaleBill), FALSE);
  gtk_widget_set_size_request(bottomFrame, 150, -1);

  gtk_frame_set_child(GTK_FRAME(topFrame), topLabel);
  gtk_frame_set_child(GTK_FRAME(bottomFrame), grid);

  // Grid Config
  //
  for (unsigned int i = 0; i < lb; i++) {
    gtk_grid_attach(GTK_GRID(grid), label[i], 0, i, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry[i], 1, i, 1, 1);
  }
  gtk_grid_attach(GTK_GRID(grid), savebtn, 0, lb, 1, 1);

  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
  gtk_widget_set_margin_start(grid, 10);
  gtk_widget_set_margin_top(grid, 5);
  for (unsigned int j = 0; j < lb; j++)
    gtk_widget_set_halign(label[j], GTK_ALIGN_START);
}

/******************
 * Purchase Entry *
 * ****************/
static void purchase_entry() {
  GtkWidget *topFrame = gtk_frame_new(NULL);
  GtkWidget *bottomFrame = gtk_frame_new(NULL);
  GtkWidget *topLabel = gtk_label_new("Purchase Entry");
  GtkWidget *grid = gtk_grid_new();
  unsigned int lb = 6;
  GtkWidget *label[lb];
  GtkWidget *entry[lb];
  GtkWidget *lblwarn = gtk_label_new(NULL);
  GtkWidget *savebtn = gtk_button_new_with_label("Save");
  const gchar *const lbs[] = {"Company",      "Product ID", "Date",
                              "Product Name", "Quantity",   "Price / Quantity"};

  for (unsigned int i = 0; i < lb; i++) {
    label[i] = gtk_label_new(lbs[i]);
  }

  entry[0] = gtk_drop_down_new_from_strings(company);
  entry[1] = gtk_drop_down_new_from_strings(product);
  for (unsigned int j = 2; j < lb; j++)
    entry[j] = gtk_entry_new();

  // Signal for save data
  g_signal_connect(savebtn, "clicked", G_CALLBACK(save_purchase_data), grid);

  // Purchase Entry Pane
  gtk_widget_set_size_request(df->hpanedPurchase, 200, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpanedPurchase), topFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpanedPurchase), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpanedPurchase), FALSE);
  gtk_widget_set_size_request(topFrame, 50, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpanedPurchase), bottomFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpanedPurchase), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpanedPurchase), FALSE);
  gtk_widget_set_size_request(bottomFrame, 150, -1);

  gtk_frame_set_child(GTK_FRAME(topFrame), topLabel);
  gtk_frame_set_child(GTK_FRAME(bottomFrame), grid);

  // Grid Config
  //
  for (unsigned int i = 0; i < lb; i++) {
    gtk_grid_attach(GTK_GRID(grid), label[i], 0, i, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry[i], 1, i, 1, 1);
  }
  gtk_grid_attach(GTK_GRID(grid), lblwarn, 2, 0, 1, 2);
  gtk_grid_attach(GTK_GRID(grid), savebtn, 0, lb, 1, 1);

  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
  gtk_widget_set_margin_start(grid, 10);
  gtk_widget_set_margin_top(grid, 5);
  for (unsigned int j = 0; j < lb; j++)
    gtk_widget_set_halign(label[j], GTK_ALIGN_START);
}

/*
 * Save Purchase Data
 */
static void save_purchase_data(GtkWidget *widget, gpointer data) {
  (void)widget;
  guint company_num = gtk_drop_down_get_selected(GTK_DROP_DOWN(gtk_grid_get_child_at(GTK_GRID(data), 1, 0)));
  guint product_num = gtk_drop_down_get_selected(GTK_DROP_DOWN(gtk_grid_get_child_at(GTK_GRID(data), 1, 1)));
  GtkWidget *lblwarn = gtk_grid_get_child_at(GTK_GRID(data), 2, 0);
  if(company_num == 0 && product_num == 0) {
    gtk_label_set_text(GTK_LABEL(lblwarn), "*Select company and product");
  } else {
    printf("%d %d\n", company_num, product_num);
    gtk_widget_set_visible(lblwarn, FALSE);
  }
}

/***********************
 * Purchase Bill entry *
 * *********************/
static void purchase_bill_entry() {
  GtkWidget *topFrame = gtk_frame_new(NULL);
  GtkWidget *bottomFrame = gtk_frame_new(NULL);
  GtkWidget *topLabel = gtk_label_new("Purchase Bill Entry");
  GtkWidget *grid = gtk_grid_new();
  unsigned int lb = 6;
  GtkWidget *label[lb];
  GtkWidget *entry[lb];
  GtkWidget *savebtn = gtk_button_new_with_label("Save");
  const gchar *const lbs[] = {"Company",      "Product ID", "Date",
                              "Product Name", "Quantity",   "Price / Quantity"};

  for (unsigned int i = 0; i < lb; i++) {
    label[i] = gtk_label_new(lbs[i]);
  }

  entry[0] = gtk_drop_down_new_from_strings(company);
  entry[1] = gtk_drop_down_new_from_strings(product);
  for (unsigned int j = 2; j < lb; j++)
    entry[j] = gtk_entry_new();

  // Purchase Bill Entry Pane
  gtk_widget_set_size_request(df->hpanedPurchaseBill, 200, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpanedPurchaseBill), topFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpanedPurchaseBill), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpanedPurchaseBill), FALSE);
  gtk_widget_set_size_request(topFrame, 50, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpanedPurchaseBill), bottomFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpanedPurchaseBill), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpanedPurchaseBill), FALSE);
  gtk_widget_set_size_request(bottomFrame, 150, -1);

  gtk_frame_set_child(GTK_FRAME(topFrame), topLabel);
  gtk_frame_set_child(GTK_FRAME(bottomFrame), grid);

  // Grid Config
  //
  for (unsigned int i = 0; i < lb; i++) {
    gtk_grid_attach(GTK_GRID(grid), label[i], 0, i, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry[i], 1, i, 1, 1);
  }
  gtk_grid_attach(GTK_GRID(grid), savebtn, 0, lb, 1, 1);

  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
  gtk_widget_set_margin_start(grid, 10);
  gtk_widget_set_margin_top(grid, 5);
  for (unsigned int j = 0; j < lb; j++)
    gtk_widget_set_halign(label[j], GTK_ALIGN_START);
}

/******************
 * Database Query *
 ******************/
typedef struct _QueryData {
  GtkWidget *dropdn[3];
  GtkWidget *entry[2];
} QueryData;

static QueryData ents;
static void query_database() {
  GtkWidget *topFrame = gtk_frame_new(NULL);
  GtkWidget *bottomFrame = gtk_frame_new(NULL);
  GtkWidget *topResultFrame = gtk_frame_new(NULL);
  GtkWidget *bottomResultFrame = gtk_frame_new(NULL);
  GtkWidget *topLable = gtk_label_new("Database Query");
  GtkWidget *grid[2];
  grid[0] = gtk_grid_new();
  grid[1] = gtk_grid_new();
  GtkWidget *label[2];
  GtkWidget *btn = gtk_button_new_with_label("Query");

  const gchar *dbs[] = {"Select database...", "Sale",          "Sale Bill",
                        "Purchase",           "Purchase Bill", NULL};

  label[0] = gtk_label_new("Start date");
  label[1] = gtk_label_new("End date");
  ents.dropdn[0] = gtk_drop_down_new_from_strings(dbs);
  ents.dropdn[1] = gtk_drop_down_new_from_strings(company);
  ents.dropdn[2] = gtk_drop_down_new_from_strings(product);
  ents.entry[0] = gtk_entry_new();
  ents.entry[1] = gtk_entry_new();

  // Database Query Pane
  gtk_widget_set_size_request(df->hpanedQuery, 200, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpanedQuery), topFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpanedQuery), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpanedQuery), FALSE);
  gtk_widget_set_size_request(topFrame, 50, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpanedQuery), bottomFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpanedQuery), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpanedQuery), FALSE);
  gtk_widget_set_size_request(bottomFrame, 150, -1);

  gtk_frame_set_child(GTK_FRAME(topFrame), topLable);
  gtk_frame_set_child(GTK_FRAME(bottomFrame), df->hpanedQueryResult);

  // Database Query Result Pane
  gtk_widget_set_size_request(df->hpanedQueryResult, 200, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpanedQueryResult), topResultFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpanedQueryResult), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpanedQueryResult), FALSE);
  gtk_widget_set_size_request(topResultFrame, 50, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpanedQueryResult), bottomResultFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpanedQueryResult), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpanedQueryResult), FALSE);
  gtk_widget_set_size_request(bottomResultFrame, 150, -1);

  gtk_frame_set_child(GTK_FRAME(topResultFrame), grid[0]);
  gtk_frame_set_child(GTK_FRAME(bottomResultFrame), grid[1]);

  // Signals
  g_signal_connect(btn, "clicked", G_CALLBACK(query_result), grid[1]);
  //
  // Grid Configuration
  gtk_grid_attach(GTK_GRID(grid[0]), ents.dropdn[0], 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid[0]), ents.dropdn[1], 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid[0]), ents.dropdn[2], 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid[0]), label[0], 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid[0]), ents.entry[0], 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid[0]), label[1], 2, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid[0]), ents.entry[1], 3, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid[0]), btn, 0, 2, 1, 1);

  gtk_grid_set_row_spacing(GTK_GRID(grid[0]), 4);
  gtk_grid_set_column_spacing(GTK_GRID(grid[0]), 5);
  gtk_widget_set_margin_start(grid[0], 5);
  gtk_widget_set_halign(label[0], GTK_ALIGN_START);
  gtk_widget_set_halign(label[1], GTK_ALIGN_START);
  gtk_widget_set_halign(btn, GTK_ALIGN_START);
  
  gtk_drop_down_set_show_arrow(GTK_DROP_DOWN(ents.dropdn[0]), TRUE);
  gtk_drop_down_set_show_arrow(GTK_DROP_DOWN(ents.dropdn[1]), TRUE);
  gtk_drop_down_set_show_arrow(GTK_DROP_DOWN(ents.dropdn[2]), TRUE);
}

/****************
 * Query Result *
 ****************/
static void query_result(GtkWidget *widget, gpointer data) {
  /*{"Sale", "Sale Bill","Purchase", "Purchase Bill"}*/
  (void)widget;
  short dbs = gtk_drop_down_get_selected(GTK_DROP_DOWN(ents.dropdn[0]));
  short com = gtk_drop_down_get_selected(GTK_DROP_DOWN(ents.dropdn[1]));
  short prod = gtk_drop_down_get_selected(GTK_DROP_DOWN(ents.dropdn[2]));

  gtk_grid_remove_row(GTK_GRID(data), 0);
  if (dbs == 0) {
    GtkWidget *err = gtk_label_new("Err: No database selected...");
    gtk_grid_attach(GTK_GRID(data), err, 0, 0, 1, 1);
  } else if (com == 0 && prod == 0) {
    printf("Database selected...\n");
  } else if (com == 0 && prod > 0) {
    printf("Product selected...\n");
  } else if (com > 0 && prod == 0) {
    printf("Company selected...\n");
  } else if (com > 0 && prod > 0) {
    printf("Both selected...\n");
  }
}

/******************
 * Login Database *
 * ****************/
static void login_database() {
  GtkWidget *topFrame = gtk_frame_new(NULL);
  GtkWidget *bottomFrame = gtk_frame_new(NULL);
  GtkWidget *topLabel = gtk_label_new("Update Database");
  GtkWidget *grid = gtk_grid_new();
  GtkWidget *uidLb = gtk_label_new("User ID");
  GtkWidget *passwdLb = gtk_label_new("Password");
  GtkWidget *uidEnt = gtk_entry_new();
  GtkWidget *passwdEnt = gtk_entry_new();
  GtkWidget *btn = gtk_button_new_with_label("Login");
  GtkWidget *msg = gtk_label_new(NULL);

  // Database Update Pane (admin)
  gtk_widget_set_size_request(df->hpanedUpdateDb, 200, -1);

  gtk_paned_set_start_child(GTK_PANED(df->hpanedUpdateDb), topFrame);
  gtk_paned_set_resize_start_child(GTK_PANED(df->hpanedUpdateDb), FALSE);
  gtk_paned_set_shrink_start_child(GTK_PANED(df->hpanedUpdateDb), FALSE);
  gtk_widget_set_size_request(topFrame, 50, -1);

  gtk_paned_set_end_child(GTK_PANED(df->hpanedUpdateDb), bottomFrame);
  gtk_paned_set_resize_end_child(GTK_PANED(df->hpanedUpdateDb), TRUE);
  gtk_paned_set_shrink_end_child(GTK_PANED(df->hpanedUpdateDb), FALSE);
  gtk_widget_set_size_request(bottomFrame, 150, -1);

  gtk_frame_set_child(GTK_FRAME(topFrame), topLabel);
  gtk_frame_set_child(GTK_FRAME(bottomFrame), grid);

  // Grid Config
  gtk_grid_attach(GTK_GRID(grid), uidLb, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), uidEnt, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), passwdLb, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), passwdEnt, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn, 1, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), msg, 2, 0, 1, 2);

  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
  gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(uidLb, GTK_ALIGN_START);
  gtk_widget_set_halign(passwdLb, GTK_ALIGN_START);
  gtk_widget_set_valign(msg, GTK_ALIGN_CENTER);

  // Signals
  g_signal_connect(btn, "clicked", G_CALLBACK(update_database), grid);
}

/*******************
 * Update Database *
 * *****************/
static void update_database(GtkWidget *widget, gpointer data) {
  GtkWidget *dblist = NULL;
  GtkWidget *grid = gtk_grid_new();
  GtkWidget *window = gtk_window_new();

  // Quering data from database...
  const char *queryusr = "select uname, passwd FROM users";
  QueryRes *resusr = dbquery(queryusr, Company);
  MYSQL_ROW row = NULL;
  const char *uid = NULL;
  const char *passwd = NULL;
  if (resusr != NULL && (row = mysql_fetch_row(resusr->result))) {
    uid = row[0];
    passwd = row[1];
  }
  free(resusr);

  gtk_window_set_title(GTK_WINDOW(window), "Update Database");
  gtk_window_set_child(GTK_WINDOW(window), grid);

  const gchar *const dbs[] = {
      "Select database...", "address",      "company", "gst",      "product",
      "purchase",           "purchasebill", "sale",    "salebill", NULL};

  dblist = gtk_drop_down_new_from_strings(dbs);

  gtk_grid_attach(GTK_GRID(grid), dblist, 0, 0, 1, 1);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(logout_db), data);
  g_signal_connect_after(dblist, "notify::selected",
                         G_CALLBACK(update_selected_db), NULL);

  // Login check
  GtkWidget *uidobj = gtk_grid_get_child_at(GTK_GRID(data), 1, 0);
  GtkWidget *passwdobj = gtk_grid_get_child_at(GTK_GRID(data), 1, 1);
  GtkWidget *msg = gtk_grid_get_child_at(GTK_GRID(data), 2, 0);
  const gchar *uidfetch = gtk_editable_get_text(GTK_EDITABLE(uidobj));
  const gchar *passwdfetch = gtk_editable_get_text(GTK_EDITABLE(passwdobj));

  if (g_strcmp0(uid, uidfetch) == 0 && g_strcmp0(passwd, passwdfetch) == 0) {
    gtk_label_set_text(GTK_LABEL(msg), "Login successfull");
    gtk_widget_set_visible(widget, FALSE);
    gtk_window_present(GTK_WINDOW(window));
  } else {
    gtk_label_set_text(GTK_LABEL(msg), "Wrong password or user id");
    gtk_window_destroy(GTK_WINDOW(window));
  }
}

/***************
 * Logout user *
 ***************/
static void logout_db(GtkWidget *widget, gpointer data) {
  (void)widget;

  GtkWidget *btn = gtk_grid_get_child_at(GTK_GRID(data), 1, 2);
  GtkWidget *msg = gtk_grid_get_child_at(GTK_GRID(data), 2, 0);
  gtk_widget_set_visible(btn, TRUE);
  gtk_label_set_text(GTK_LABEL(msg), "Logged out");
}

/****************************
 * Update selected database *
 ****************************/
static void update_selected_db(GtkWidget *widget, gpointer data) {
  (void)data;
  short i = gtk_drop_down_get_selected(GTK_DROP_DOWN(widget));
  printf("%d\n", i);
}

/******************************
 * Destroy alocated memory... *
 ******************************/
static void free_memory(GtkWidget *widget, gpointer data) {
  (void)widget;
  (void)data;
  free(company);
  free(product);
  free(df);
}

/********************
 * Activate Windows.*
 ********************/
void activate(GtkApplication *app, gpointer data) {
  (void)data;
  GtkWidget *window = NULL;
  df = (DataFort *)malloc(sizeof(DataFort));

  // Window configuration
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Data Fort");
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);

  createWidgets();

  gtk_window_set_child(GTK_WINDOW(window), df->hpane);

  paneConfig();
  stackConfig();
  company_product_list();
  sale_entry();
  sale_bill_entry();
  purchase_entry();
  purchase_bill_entry();
  query_database();
  login_database();

  // Signals
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(free_memory), NULL);
  // Show window
  gtk_window_present(GTK_WINDOW(window));
}
