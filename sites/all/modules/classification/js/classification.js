
$.os = {
    name: (/(win|mac|linux|sunos|solaris|iphone)/.exec(navigator.platform.toLowerCase()) || [u])[0].replace('sunos', 'solaris')
};

$(function() {

  //initialize a Juggernaut connection
  var jug = new Juggernaut({
    'host' : Drupal.settings.juggernaut_host,
    'port' : Drupal.settings.juggernaut_port
  });
  jug.on("connect", function() { });
  jug.on("disconnect", function() { });
  jug.on("reconnect", function() { });

  //override the ActiveX jQuery settings
  $.ajaxSetup({
    xhr:function() { return new XMLHttpRequest(); }
  });

  //disable the enter key
  $('.classification-search-form-text').bind("keypress", function(e) {
    if (e.keyCode == 13) return false;
  });

  //Drupal behaviours cache
  var abcd = [];

  //set-up some in-scope variables
  var primary_classification = Drupal.settings.classification_primary;
  var deleted_names = Drupal.settings.classification_deletions;
  var lost_found = Drupal.settings.classification_lost_found;
  var vid_ranks = Drupal.settings.classification_relations;
  var vid_vernaculars = Drupal.settings.classification_vernaculars;
  var classification_open = Drupal.settings.classification_open;
  var tid = Drupal.settings['classification_tags']['tid'];

  var classification_logging = Drupal.settings.classification_show_messages;

  if(Drupal.settings['classification_tags']['ancestry']) {
    var ancestors = Drupal.settings['classification_tags']['ancestry'].split(",");  
  }
  $('#tabs-wrapper').css('width','50%');

  if(classification_logging) {
      //create the logs section
      $('body').append('<div class="classification-logs-wrapper"><div class="classification-logs-resizer"><span>&nbsp;</span></div><div class="classification-logs"></div></div>');

      //make a resizer
      var logsPanelmov = false;
      var logsPanelorig = 55;
      var logsPanelnew = 0;
      var logsPanelpos = 0;
      $('.classification-logs-resizer').mousedown(function(e) {
        logsPanelmov = true;
        logsPanelpos = e.pageY;
      });
      $(document).mousemove(function(e) {
        if(logsPanelmov == false) return;
        $('.classification-logs-wrapper').height(logsPanelpos-e.pageY+logsPanelorig+logsPanelnew);
        if($('.classification-logs-wrapper').height() <= logsPanelorig) {
          $('.classification-logs-wrapper').height(logsPanelorig);
        }
      });
      $(document).mouseup(function(e) {
        logsPanelmov = false;
        logsPanelpos = e.pageY;
        logsPanelnew = $('.classification-logs-wrapper').height()-logsPanelorig;
      });
  }

  TREE = {};

  TREE.jsTree = $.tree_create();

  //holders for cut/copied/pasted nodes
  HOLDER = { 'cut_nodes' : false, 'copy_nodes' : false, 'tree_obj' : false };

  var cookie = tid ? false : { 'prefix' : 'primary_' };

  //Initialize the primary classification
  TREE.jsTree.init($("#classification_tree_" + primary_classification), {
        vid     : primary_classification,
        data    : {
            type    : "xml_flat", // ENUM [json, xml_flat, xml_nested, predefined]
            method  : "GET",        // HOW TO REQUEST FILES
            async   : true,        // BOOL - async loading onopen
            async_data : function (NODE) { return { tid : $(NODE).attr("id") || 0 } }, // PARAMETERS PASSED TO SERVER
            url     : Drupal.settings.classification_callback_jstree + primary_classification + "/xml/",        // FALSE or STRING - url to document to be used (async or not)
            json    : false,        // FALSE or OBJECT if type is JSON and async is false - the tree dump as json
            xml     : false         // FALSE or STRING
        },
        selected    : tid,        // FALSE or STRING or ARRAY
        opened      : ancestors,           // ARRAY OF INITIALLY OPENED NODES
        languages   : [],           // ARRAY of string values (which will be used as CSS classes - so they must be valid)
        path        : Drupal.settings.jstree_path,        // FALSE or STRING (if false - will be autodetected)
        cookies     : cookie,        // FALSE or OBJECT (prefix, open, selected, opts - from jqCookie - expires, path, domain, secure)
        ui      : {
            dots        : true,     // BOOL - dots or no dots
            rtl         : false,    // BOOL - is the tree right-to-left
            animation   : 0,        // INT - duration of open/close animations in miliseconds
            hover_mode  : true,     // SHOULD get_* functions chage focus or change hovered item
            scroll_spd  : 4,
            theme_path  : Drupal.settings.jstree_path + 'themes/',    // Path to themes
            theme_name  : 'classification',// Name of theme
            context     : [
                {
                    id      : "metadata",
                    label   : "Show name data", 
                    icon    : "application_form.gif",
                    visible : function (NODE, TREE_OBJ) { if($('#classification_name_metadata').css('display') == 'block') return false; }, 
                    action  : function (NODE, TREE_OBJ) {
                      if($('#classification_name_metadata').css('display') != 'block') {
                        TREE.show_metadata('left');
                      }
                    }
                },
                "separator",
                { 
                    id      : "rename",
                    label   : "Rename", 
                    icon    : "page_white_edit.gif",
                    visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("renameable", NODE); }, 
                    action  : function (NODE, TREE_OBJ) {
                      TREE_OBJ.rename();
                      $(NODE).find("input").bind("keyup", function() {
                        $('#metadata_taxon_title:span').text($(this).val());
                      });
                    } 
                },
                {
                    id      : "create",
                    label   : "New child", 
                    icon    : "page_white_add.gif",
                    visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("creatable", NODE); }, 
                    action  : function (NODE, TREE_OBJ) {
                      TREE_OBJ.create(false, TREE_OBJ.get_node(NODE));
                      $(NODE).find("input").bind("keyup", function() {
                        $('#metadata_taxon_title:span').text($(this).val());
                      });
                    } 
                },
                {
                    id      : "bulk_create",
                    label   : "Bulk insert", 
                    icon    : "table_add.gif",
                    visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("creatable", NODE); }, 
                    action  : function (NODE, TREE_OBJ) { 
                      TREE.bulk_insert(NODE,TREE_OBJ);
                    } 
                },
                "separator",
                { 
                    id      : "cut",
                    label   : "Cut", 
                    icon    : "cut.gif",
                    visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("deletable", NODE); }, 
                    action  : function (NODE, TREE_OBJ) { TREE.cut(NODE,TREE_OBJ); } 
                },
                { 
                    id      : "copy",
                    label   : "Copy", 
                    icon    : "page_white_copy.gif",
                    visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("copyable", NODE); }, 
                    action  : function (NODE, TREE_OBJ) { TREE.copy(NODE,TREE_OBJ); } 
                },
                { 
                    id      : "paste",
                    label   : "Paste", 
                    icon    : "page_white_paste.gif",
                    visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("creatable", NODE); }, 
                    action  : function (NODE, TREE_OBJ) { TREE.paste(NODE,TREE_OBJ); } 
                },
                "separator",
                { 
                    id      : "delete",
                    label   : "Delete",
                    icon    : "bin_closed.gif",
                    visible : function (NODE, TREE_OBJ) { var ok = true; $.each(NODE, function () { if(TREE_OBJ.check("deletable", this) == false) ok = false; return false; }); return ok; }, 
                    action  : function (NODE, TREE_OBJ) { TREE.delete_name_warning(NODE,TREE_OBJ); } 
                }
            ]
        },
        rules   : {
            multiple    : false,    // FALSE | CTRL | ON - multiple selection off/ with or without holding Ctrl
            metadata    : "mdata",    // FALSE or STRING - attribute name (use metadata plugin)
            type_attr   : "rel",    // STRING attribute name (where is the type stored if no metadata)
            multitree   : true,    // BOOL - is drag n drop between trees allowed
            createat    : "top", // STRING (top or bottom) new nodes get inserted at top or bottom
            use_inline  : true,    // CHECK FOR INLINE RULES - REQUIRES METADATA
            clickable   : "all",    // which node types can the user select | default - all
            renameable  : "all",    // which node types can the user select | default - all
            deletable   : "all",    // which node types can the user delete | default - all
            creatable   : "all",    // which node types can the user create in | default - all
            draggable   : "all",   // which node types can the user move | default - none | "all"
            dragrules   : "all",    // what move operations between nodes are allowed | default - none | "all"
            drag_copy   : false,    // FALSE | CTRL | ON - drag to copy off/ with or without holding Ctrl
            droppable   : [],
            drag_button : "left"
        },
        lang : {
            new_node    : Drupal.t("New taxon"),
            loading     : Drupal.t("Loading ...")
        },
        callback    : {             // various callbacks to attach custom logic to
            // before focus  - should return true | false
            beforechange: function(NODE,TREE_OBJ) { return true },
            beforeopen  : function(NODE,TREE_OBJ) { return true },
            beforeclose : function(NODE,TREE_OBJ) { return true },
            // before move   - should return true | false
            beforemove  : function(NODE,REF_NODE,TYPE,TREE_OBJ) {
                var source = $(NODE).parents(".tree:eq(0)").attr("id").split("_");
                var dest = $(REF_NODE).parents(".tree:eq(0)").attr("id").split("_");

                var status = TREE.check_status();
                var status_destination = false;

                if(status) {
                  if(source[2] != dest[2] && source[2] != primary_classification && source[2] != deleted_names && source[2] != lost_found){
                    status_destination = TREE.check_destination(dest[2], TREE.get_id(REF_NODE));
                    if(status_destination) {
                      TREE.copy_name(NODE,REF_NODE,TYPE,TREE_OBJ);
                    }
                    return false;
                  }
                  else if(source[2] != dest[2] && source[2] == primary_classification) {
                    alert(Drupal.t("Sorry, names may not be moved from your primary classification into an alternate classification"));
                    return false;
                  }
                  else if(source[2] == dest[2] && source[2] == deleted_names) {
                    alert(Drupal.t("Sorry, names within deleted names cannot be moved"));
                    return false;
                  }
                  else if(source[2] == dest[2] && source[2] == lost_found) {
                    alert(Drupal.t("Sorry, names within lost and found cannot be moved"));
                    return false;
                  }
                  else if(source[2] != dest[2] && (source[2] == deleted_names || source[2] == lost_found)) {
                    status_destination = TREE.check_destination(dest[2], TREE.get_id(REF_NODE));
                    if(status_destination) {
                        $(NODE).removeClass("hascontent").removeClass("nocontent").find("li.hascontent").each(function() {
                          $(this).removeClass("hascontent").removeClass("nocontent");
                        });
                        return true;
                    }
                    else {
                      return false;
                    }
                  }
                  else {
                    status_destination = TREE.check_destination(dest[2], TREE.get_id(REF_NODE));
                    if(status_destination) {
                      return true;
                    }
                    else {
                      return false;
                    }
                  }
                }
                else {
                  return false;
                }
            },
            beforecreate: function(NODE,REF_NODE,TYPE,TREE_OBJ) {
                var parent  = (typeof REF_NODE == 'undefined' || typeof TREE_OBJ.selected == 'undefined' || TREE_OBJ.container.find('#'+REF_NODE.id).length == 0) ? -1 : REF_NODE;
                TREE.add_name(NODE,parent,TYPE,TREE_OBJ);
                return true;
            }, 
            // before rename - should return true | false
            beforerename: function(NODE,LANG,TREE_OBJ) { return true; },
            beforedelete: function(NODE,TREE_OBJ) {
              var del = TREE.delete_name(NODE,primary_classification);
              if(del) return true;
            },
            onJSONdata  : function(DATA,TREE_OBJ) { return DATA; },
            onselect    : function(NODE,TREE_OBJ) {
                  if($('#classification_name_metadata:visible').length > 0) {
                    TREE.get_metadata(NODE,TREE_OBJ);
                  }
            },                  // node selected
            ondeselect  : function(NODE,TREE_OBJ) { },                  // node deselected
            onchange    : function(NODE,TREE_OBJ) { },
            onrename    : function(NODE,LANG,TREE_OBJ,RB) {
              TREE.edit_name(NODE,TREE_OBJ);
            },              // node renamed ISNEW - TRUE|FALSE, current language
            onmove      : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) {
              TREE.move_name(NODE,REF_NODE,TYPE,TREE_OBJ);
            }, // move completed (TYPE is BELOW|ABOVE|INSIDE)
            oncopy      : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) { }, // copy completed (TYPE is BELOW|ABOVE|INSIDE)
            oncreate    : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) {
                if(!TREE_OBJ.selected) {
                    TREE_OBJ.select_branch.call(TREE_OBJ, NODE);
                    TREE_OBJ.rename.call(TREE_OBJ,NODE);
                  }
            }, // node created, parent node (TYPE is createat)
            ondelete    : function(NODE, TREE_OBJ,RB) { },                  // node deleted
            onopen      : function(NODE, TREE_OBJ) { },                 // node opened
            onopen_all  : function(TREE_OBJ) { },                       // all nodes opened
            onclose     : function(NODE, TREE_OBJ) { },                 // node closed
            error       : function(TEXT, TREE_OBJ) { },                 // error occured
            // double click on node - defaults to open/close & select
            ondblclk    : function(NODE, TREE_OBJ) { 
              TREE_OBJ.toggle_branch.call(TREE_OBJ, NODE); 
              TREE_OBJ.select_branch.call(TREE_OBJ, NODE);
              TREE_OBJ.rename.call(TREE_OBJ,NODE);
              $(NODE).find("input").bind("keyup", function() {
                $('#metadata_taxon_title:span').text($(this).val());
              });
            },
            // right click - to prevent use: EV.preventDefault(); EV.stopPropagation(); return false
            onrgtclk    : function(NODE, TREE_OBJ, EV) { },
            onload      : function(TREE_OBJ) { },
            onfocus     : function(TREE_OBJ) { },
            ondrop      : function(NODE,REF_NODE,TYPE,TREE_OBJ) {}
        }
  });

    //build array of alternate (and deleted names) classifications
    TREE.get_tree = function(vid) {
        var _this = this;
    
        $('.classification_trees').hide();
        if(TREE.jsTree[vid] || vid == 0) {
            $('#classification_tree_' + vid).show();
            return;
        }
        var _tree = '<div id="classification_tree_' + vid + '" class="classification_trees"></div>';
        $('#classification_tree_alternate').append(_tree);
        $('#classification_tree_' + vid).show();

        TREE.jsTree[vid] = $.tree_create();

        var alt_cookie = tid ? false : { 'prefix' : 'secondary_' + vid };

        TREE.jsTree[vid].init($("#classification_tree_" + vid), {
            vid     : vid,
            data    : {
                type    : "xml_flat", // ENUM [json, xml_flat, xml_nested, predefined]
                method  : "GET",        // HOW TO REQUEST FILES
                async   : true,        // BOOL - async loading onopen
                async_data : function (NODE) { return { tid : $(NODE).attr("id") || 0 } }, // PARAMETERS PASSED TO SERVER
                url     : Drupal.settings.classification_callback_jstree + vid + "/xml/",        // FALSE or STRING - url to document to be used (async or not)
                json    : false,        // FALSE or OBJECT if type is JSON and async is false - the tree dump as json
                xml     : false         // FALSE or STRING
            },
            selected    : false,        // FALSE or STRING or ARRAY
            opened      : [],           // ARRAY OF INITIALLY OPENED NODES
            languages   : [],           // ARRAY of string values (which will be used as CSS classes - so they must be valid)
            path        : Drupal.settings.jstree_path,        // FALSE or STRING (if false - will be autodetected)
            cookies     : alt_cookie,        // FALSE or OBJECT (prefix, open, selected, opts - from jqCookie - expires, path, domain, secure)
            ui      : {
                dots        : true,     // BOOL - dots or no dots
                rtl         : false,    // BOOL - is the tree right-to-left
                animation   : 0,        // INT - duration of open/close animations in miliseconds
                hover_mode  : true,     // SHOULD get_* functions chage focus or change hovered item
                scroll_spd  : 4,
                theme_path  : Drupal.settings.jstree_path + 'themes/',    // Path to themes
                theme_name  : 'classification_alternate',// Name of theme
                context     : [
                    {
                        id      : "metadata",
                        label   : "Show name data", 
                        icon    : "application_form.gif",
                        visible : function (NODE, TREE_OBJ) { if($('#classification_name_metadata').css('display') == 'block') return false; }, 
                        action  : function (NODE, TREE_OBJ) {
                          if($('#classification_name_metadata').css('display') != 'block') {
                            TREE.show_metadata('right');
                          }
                        }
                    },
                    "separator",
                    {
                        id      : "create",
                        label   : "New child", 
                        icon    : "page_white_add.gif",
                        visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("creatable", NODE); }, 
                        action  : function (NODE, TREE_OBJ) { 
                          TREE_OBJ.create(false, TREE_OBJ.get_node(NODE));
                          $(NODE).find("input").bind("keyup", function() {
                            $('#metadata_taxon_title:span').text($(this).val());
                          });
                        } 
                    },
                    {
                        id      : "bulk_create",
                        label   : "Bulk insert", 
                        icon    : "table_add.gif",
                        visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("creatable", NODE); }, 
                        action  : function (NODE, TREE_OBJ) { 
                          TREE.bulk_insert(NODE,TREE_OBJ);
                        } 
                    },
                    { 
                        id      : "cut",
                        label   : "Cut", 
                        icon    : "cut.gif",
                        visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("deletable", NODE); }, 
                        action  : function (NODE, TREE_OBJ) { TREE.cut_alt(NODE,TREE_OBJ); } 
                    },
                    { 
                        id      : "copy",
                        label   : "Copy", 
                        icon    : "page_white_copy.gif",
                        visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("copyable", NODE); }, 
                        action  : function (NODE, TREE_OBJ) { TREE.copy_alt(NODE,TREE_OBJ); } 
                    },
                    { 
                        id      : "paste",
                        label   : "Paste", 
                        icon    : "page_white_paste.gif",
                        visible : function (NODE, TREE_OBJ) { if(NODE.length != 1) return false; return TREE_OBJ.check("creatable", NODE); }, 
                        action  : function (NODE, TREE_OBJ) { TREE.paste_alt(NODE,TREE_OBJ); } 
                    },
                    "separator",
                    { 
                        id      : "delete",
                        label   : "Delete",
                        icon    : "bin_closed.gif",
                        visible : function (NODE, TREE_OBJ) { var ok = true; if(TREE_OBJ.settings.vid == deleted_names) { ok = false; } $.each(NODE, function () { if(TREE_OBJ.check("deletable", this) == false) ok = false; return false; }); return ok; }, 
                        action  : function (NODE, TREE_OBJ) { TREE.delete_name_warning_alt(NODE,TREE_OBJ); } 
                    }
                ]
            },
            rules   : {
                multiple    : false,    // FALSE | CTRL | ON - multiple selection off/ with or without holding Ctrl
                metadata    : "mdata",    // FALSE or STRING - attribute name (use metadata plugin)
                type_attr   : "rel",    // STRING attribute name (where is the type stored if no metadata)
                multitree   : true,    // BOOL - is drag n drop between trees allowed
                createat    : "top", // STRING (top or bottom) new nodes get inserted at top or bottom
                use_inline  : true,    // CHECK FOR INLINE RULES - REQUIRES METADATA
                clickable   : "all",    // which node types can the user select | default - all
                renameable  : "all",    // which node types can the user select | default - all
                deletable   : "all",    // which node types can the user delete | default - all
                creatable   : "all",    // which node types can the user create in | default - all
                draggable   : "all",   // which node types can the user move | default - none | "all"
                dragrules   : "all",    // what move operations between nodes are allowed | default - none | "all"
                drag_copy   : "ON",    // FALSE | CTRL | ON - drag to copy off/ with or without holding Ctrl
                droppable   : [],
                drag_button : "left"
            },
            lang : {
                new_node    : Drupal.t("New taxon"),
                loading     : Drupal.t("Loading ...")
            },
            callback    : {
                // before focus  - should return true | false
                beforechange: function(NODE,TREE_OBJ) { return true },
                beforeopen  : function(NODE,TREE_OBJ) { return true },
                beforeclose : function(NODE,TREE_OBJ) { return true },
                // before move   - should return true | false
                beforemove  : function(NODE,REF_NODE,TYPE,TREE_OBJ) {
                    var source = $(NODE).parents(".tree:eq(0)").attr("id").split("_");
                    var dest = $(REF_NODE).parents(".tree:eq(0)").attr("id").split("_");

                    var status = TREE.check_status();
                    var status_destination = false;

                    if(status) {
                      if(source[2] != dest[2] && source[2] != primary_classification && source[2] != deleted_names && source[2] != lost_found) {
                        status_destination = TREE.check_destination(dest[2], TREE.get_id(REF_NODE));
                        if(status_destination) {
                          TREE.copy_name(NODE,REF_NODE,TYPE,TREE_OBJ);
                        }
                        return false;
                      }
                      else if(source[2] != dest[2] && source[2] == primary_classification) {
                        alert(Drupal.t("Sorry, names may not be moved from your primary classification into an alternate classification"));
                        return false;
                      }
                      else if(source[2] == dest[2] && source[2] == deleted_names) {
                        alert(Drupal.t("Sorry, names within deleted names cannot be moved"));
                        return false;
                      }
                      else if(source[2] == dest[2] && source[2] == lost_found) {
                        alert(Drupal.t("Sorry, names within lost and found cannot be moved"));
                        return false;
                      }
                      else if(source[2] != dest[2] && (source[2] == deleted_names || source[2] == lost_found)) {
                        status_destination = TREE.check_destination(dest[2], TREE.get_id(REF_NODE));
                        if(status_destination) {
                            $(NODE).removeClass("hascontent").removeClass("nocontent").find("li.hascontent").each(function() {
                              $(this).removeClass("hascontent").removeClass("nocontent");
                            });
                            return true;
                        }
                        else {
                          return false;
                        }
                      }
                      else {
                        status_destination = TREE.check_destination(dest[2], TREE.get_id(REF_NODE));
                        if(status_destination) {
                          return true;
                        }
                        else {
                          return false;
                        }
                      }
                    }
                    else {
                      return false;
                    }
                },
                beforecreate: function(NODE,REF_NODE,TYPE,TREE_OBJ) {
                    var parent  = (typeof REF_NODE == 'undefined' || typeof TREE_OBJ.selected == 'undefined' || TREE_OBJ.container.find('#'+REF_NODE.id).length == 0) ? -1 : REF_NODE;
                    TREE.add_name(NODE,parent,TYPE,TREE_OBJ);
                    return true;
                }, 
                // before rename - should return true | false
                beforerename: function(NODE,LANG,TREE_OBJ) { return true; },
                beforedelete: function(NODE,TREE_OBJ) {
                  var del = TREE.delete_name(NODE,vid);
                  if(del) return true;
                },
                onJSONdata  : function(DATA,TREE_OBJ) { return DATA; },
                onselect    : function(NODE,TREE_OBJ) {
                  if($('#classification_name_metadata:visible').length > 0) {
                    TREE.get_metadata(NODE,TREE_OBJ);
                  }
                },                  // node selected
                ondeselect  : function(NODE,TREE_OBJ) { },                  // node deselected
                onchange    : function(NODE,TREE_OBJ) { },                  // focus changed
                onrename    : function(NODE,LANG,TREE_OBJ,RB) {
                  TREE.edit_name(NODE,TREE_OBJ);
                },              // node renamed ISNEW - TRUE|FALSE, current language
                onmove      : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) {
                  TREE.move_name(NODE,REF_NODE,TYPE,TREE_OBJ);
                }, // move completed (TYPE is BELOW|ABOVE|INSIDE)
                oncopy      : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) { }, // copy completed (TYPE is BELOW|ABOVE|INSIDE)
                oncreate    : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) {
                  if(!TREE_OBJ.selected) {
                    TREE_OBJ.select_branch.call(TREE_OBJ, NODE);
                    TREE_OBJ.rename.call(TREE_OBJ,NODE);
                  }
                }, // node created, parent node (TYPE is createat)
                ondelete    : function(NODE, TREE_OBJ,RB) { },                  // node deleted
                onopen      : function(NODE, TREE_OBJ) { },                 // node opened
                onopen_all  : function(TREE_OBJ) { },                       // all nodes opened
                onclose     : function(NODE, TREE_OBJ) { },                 // node closed
                error       : function(TEXT, TREE_OBJ) { },                 // error occured
                // double click on node - defaults to open/close & select
                ondblclk    : function(NODE, TREE_OBJ) { 
                  TREE_OBJ.toggle_branch.call(TREE_OBJ, NODE); 
                  TREE_OBJ.select_branch.call(TREE_OBJ, NODE);
                  TREE_OBJ.rename.call(TREE_OBJ,NODE);
                  $(NODE).find("input").bind("keyup", function() {
                    $('#metadata_taxon_title:span').text($(this).val());
                  });
                },
                // right click - to prevent use: EV.preventDefault(); EV.stopPropagation(); return false
                onrgtclk    : function(NODE, TREE_OBJ, EV) { },
                onload      : function(TREE_OBJ) { },
                onfocus     : function(TREE_OBJ) { },
                ondrop      : function(NODE,REF_NODE,TYPE,TREE_OBJ) {}
            }           
      });
    };

    /*
    * Callback function to check the status of the tree (i.e. checke if it is locked or unlocked)
    */
    TREE.check_status = function() {
        var _this = this;
        var status = false;
        $.ajax({
          type: "GET",
          url: Drupal.settings.classification_callback_base_url + "/check_status/",
          dataType: "json",
          async: false,
          success: function(data) {
              if(data.status == 'ready') {
                status = true;
              }
              else {
                 status = false;
                 var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
                 message += data.message + '<br>';
                 message += '<input type="submit" id="classification_ok" class="form-submit" value="OK">';
                 message += '</div>';
                 _this.show_message(message);
                 $('#classification_ok').click(function() { _this.hide_message(); return false; });
              } 
          },
          error: function(data) {
              status = false;
          }
        });
        return status;
    };

    /*
    * Callback function to check the destination and make sure the parent still exists
    */
    TREE.check_destination = function(vid, new_parent) {
      var _this = this;
      var status = false;
      $.ajax({
        type: "GET",
        url: Drupal.settings.classification_callback_base_url + "/check_destination/" + vid + "/" + new_parent,
        dataType: "json",
        async: false,
        success: function(data) {
            if(data.status == 'ready') {
                status = true;
            }
            else {
                status = false;
                var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
                message += data.message + '<br>';
                message += '<input type="submit" id="classification_ok" class="form-submit" value="OK">';
                message += '</div>';
                _this.show_message(message);
                $('#classification_ok').click(function() { _this.hide_message(); return false; });
            }
        },
        error: function(data) {
            status = false;
        }
      });
      return status;
    };

    /*
    * Callback function to count the number of immediate children
    */
    TREE.count_children = function(vid, parent) {
      var _this = this;
      var status = false;
      $.ajax({
        type: "GET",
        url: Drupal.settings.classification_callback_base_url + "/count_children/" + vid + "/" + parent + "/json",
        dataType: "json",
        async: false,
        success: function(data) {
            if(data.count > 0) {
                status = true;
            }
            else {
                status = false;
            }
        },
        error: function(data) {
            status = false;
        }
      });
      return status;
    };

    /*
    * Get the identifier for a node
    */
    TREE.get_id = function(NODE) {
        var node = $(NODE).attr("id");
        return node.replace("n","");
    };

    /*
    * Get the text of a node
    */
    TREE.get_content = function(NODE) {
        var node = $(NODE).attr("id");
        return $('#' + node + ' a:first').text();
    };

    /*
    * Create a child node in the primary classification
    */
    TREE.create_child = function() {
      if(typeof this.jsTree.selected == "undefined" || this.jsTree.container.find('#'+$(this.jsTree.selected).attr("id")).length == 0) {
        this.jsTree.create({"attributes":{"mdata":"{\"type\":\"\"}"}, "data":{"title":this.jsTree.settings.lang.new_node}}, -1, "before");
      }
      else {
        this.jsTree.create(false, (this.jsTree.container.find("li").size() == 0) ? -1 : false);
      }

      $('span.clicked').find("input").bind("keyup", function() {
        $('#metadata_taxon_title:span').text($(this).val());
      });
    };
    
    /*
    * Create a child node in an alternate classification
    */
    TREE.create_child_alt = function() {
      if(typeof this.jsTree[$('.right_tree').attr("rel")].selected == "undefined" || this.jsTree[$('.right_tree').attr("rel")].container.find('#'+$(this.jsTree[$('.right_tree').attr("rel")].selected).attr("id")).length == 0) {
        this.jsTree[$('.right_tree').attr("rel")].create({"attributes":{"mdata":"{\"type\":\"\"}"}, "data":{"title":this.jsTree[$('.right_tree').attr("rel")].settings.lang.new_node}}, -1, "before");
      }
      else {
        this.jsTree[$('.right_tree').attr("rel")].create(false, (this.jsTree[$('.right_tree').attr("rel")].container.find("li").size() == 0) ? -1 : false);
      }
    
      $('span.clicked').find("input").bind("keyup", function() {
        $('#metadata_taxon_title:span').text($(this).val());
      });
    };
    
    /*
    * Bulk insert (i.e. from textarea box) taxon names under a chosen parent
    */
    TREE.bulk_insert = function(NODE, TREE_OBJ) {
        
      if(!TREE_OBJ) {
        TREE_OBJ = TREE.jsTree;
        NODE = TREE_OBJ.selected;
      }
    
      if(NODE && !TREE_OBJ.check("creatable", NODE)) { return; }

      var _this = this;
      var message = '<div id="bulk-insert">';
      message += '<div id="bulk-insert-content">';
      message += '<div class="messages warning">' + Drupal.t('Bulk insert names under !name. One name per line and limit !limit names.', {'!name' : (_this.get_content(NODE)) ? _this.get_content(NODE) : Drupal.t('root'), '!limit' : addCommas(Drupal.settings.classification_import_copypaste_limit)}) + '</div>';
      message += '<textarea id="edit-name-list" class="form-textarea resizable" name="children" rows="5" cols="30"></textarea>';
      message += '<div style="margin-top:10px"><input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('Submit') + '">';
      message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
      message += '</div></div>';
      message += '</div>';
      this.show_message(message);
      Drupal.behaviors.textarea('#bulk-insert');
      $('#classification_ok').click(function() {
        var children = $('#edit-name-list').val();
        if(children != '') {
            $('#classification-message').html('<div class="messages-throbber">' + Drupal.t('Saving names...') + '</div>');
            $.post(Drupal.settings.classification_callback_base_url + "/bulkadd", {'vid' : TREE_OBJ.settings.vid, 'parent' : (_this.get_content(NODE)) ? _this.get_id(NODE) : 0, 'children' : children}, function(data) {
              switch(data.status) {
                case 'bulkadded':
                  var message_status = 'status';
                break;
            
                case 'failed':
                  var message_status = 'error';
                break;
              }
              var returned_message = '<div class="messages ' + message_status + ' classification-warning" style="margin:0px auto">' + data.message;
              returned_message += (message_status == 'error') ? '<div style="margin-top:10px"><input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '"></div>' : '';
              returned_message += '</div>';

              if(message_status == 'status') {
                (_this.get_content(NODE)) ? TREE_OBJ.refresh(NODE) : TREE_OBJ.refresh();
                _this.hide_message();
                _this.display_log(data);
              }
              else {
                $('#classification-message').html(returned_message);
              }
              $('#classification_ok').click(function() {
                _this.hide_message();
              });
            }, 'json'); 
        }
      });
      $('#classification_cancel').click(function() { 
        _this.hide_message();
      });
    };
    
    /*
    * Bulk insert (i.e. from textarea box) names under a chosen parent in an alternate classification
    */
    TREE.bulk_insert_alt = function(NODE, TREE_OBJ) {
      if(!NODE || !TREE_OBJ) {
        var NODE = TREE.jsTree[$(".right_tree").attr("rel")].selected;
        var TREE_OBJ = TREE.jsTree[$(".right_tree").attr("rel")];
      }
      this.bulk_insert(NODE, TREE_OBJ);
    };
    
    /*
    * Rename a node in the primary classification
    */
    TREE.rename = function() {
      TREE.jsTree.rename();
      $('a.clicked').parent().find("input").bind("keyup", function() {
        $('#metadata_taxon_title:span').text($(this).val());
      });
    };
    
    /*
    * Rename a node in an alternate classification
    */
    TREE.rename_alt = function() {
      TREE.jsTree[$('.right_tree').attr("rel")].rename();
      $('a.clicked').parent().find("input").bind("keyup", function() {
        $('#metadata_taxon_title:span').text($(this).val());
      });
    };
    
    /*
    * Cut a node in the primary classification
    */
    TREE.cut = function(NODE,TREE_OBJ) {
      if(!NODE || !TREE_OBJ) {
        var NODE = TREE.jsTree.selected;
        var TREE_OBJ = TREE.jsTree; 
      }
      HOLDER.copy_nodes = false;
      HOLDER.cut_nodes = NODE;
      HOLDER.tree_obj = TREE_OBJ;
    
      TREE.jsTree.cut();
    };
    
    /*
    * Cut a node in an alternate classification
    */
    TREE.cut_alt = function(NODE,TREE_OBJ) {
      if(!NODE || !TREE_OBJ) {
        var NODE = TREE.jsTree[$(".right_tree").attr("rel")].selected;
        var TREE_OBJ = TREE.jsTree[$(".right_tree").attr("rel")];
      }
      HOLDER.copy_nodes = false;
      HOLDER.cut_nodes = NODE;
      HOLDER.tree_obj = TREE_OBJ;
    
      TREE.jsTree[$(".right_tree").attr("rel")].cut();
    };
    
    /*
    * Copy a node in the primary classification
    */
    TREE.copy = function(NODE,TREE_OBJ) {
      if(!NODE || !TREE_OBJ) {
        var NODE = TREE.jsTree.selected;
        var TREE_OBJ = TREE.jsTree; 
      }
      HOLDER.copy_nodes = NODE;
      HOLDER.cut_nodes = false;
      HOLDER.tree_obj = TREE_OBJ;
    
      TREE.jsTree.copy();
    };
    
    /*
    * Copy a node in an alternate classification
    */
    TREE.copy_alt = function(NODE,TREE_OBJ) {
      if(!NODE || !TREE_OBJ) {
        var NODE = TREE.jsTree[$(".right_tree").attr("rel")].selected;
        var TREE_OBJ = TREE.jsTree[$(".right_tree").attr("rel")];   
      }
      HOLDER.copy_nodes = NODE;
      HOLDER.cut_nodes = false;
      HOLDER.tree_obj = TREE_OBJ;
    
      TREE.jsTree[$(".right_tree").attr("rel")].copy();
    };
    
    /*
    * Paste a node in the primary classification
    */
    TREE.paste = function(NODE,TREE_OBJ) {

      var nodes = HOLDER.copy_nodes ? HOLDER.copy_nodes : HOLDER.cut_nodes;

      if(!NODE || !TREE_OBJ) {
        var NODE = TREE.jsTree.selected;
        var TREE_OBJ = TREE.jsTree;
      }
    
      if(HOLDER.copy_nodes && HOLDER.tree_obj.settings.vid == TREE_OBJ.settings.vid) {
        alert(Drupal.t("To prevent duplicate names, you cannot copy and paste within a classification"));   
      }
      else {
        TREE.jsTree.paste(NODE,"inside");
        
      }
      HOLDER.cut_nodes = false;
      HOLDER.copy_nodes = false;
      HOLDER.tree_obj = false;
    };

    /*
    * Paste a node in an alternate classification
    */
    TREE.paste_alt = function(NODE,TREE_OBJ) {

      var nodes = HOLDER.copy_nodes ? HOLDER.copy_nodes : HOLDER.cut_nodes;

      if(!NODE || !TREE_OBJ) {
        var NODE = TREE.jsTree[$(".right_tree").attr("rel")].selected;
        var TREE_OBJ = TREE[$(".right_tree").attr("rel")].jsTree;
      }

      if(HOLDER.copy_nodes && HOLDER.tree_obj.settings.vid == TREE_OBJ.settings.vid) {
        alert(Drupal.t("To prevent duplicate names, you cannot copy and paste within your classification"));    
      }
      else {
        TREE.jsTree[$(".right_tree").attr("rel")].paste(NODE,"inside");     
      }
    
      HOLDER.cut_nodes = false;
      HOLDER.copy_nodes = false;
      HOLDER.tree_obj = false;
    };
    
    /*
    * Refresh the primary classification
    */
    TREE.refresh = function() {
      TREE.jsTree.refresh();
    };

    /*
    * Refresh an alternate classification
    */
    TREE.refresh_alt = function() {
      TREE.jsTree[$('.right_tree').attr("rel")].refresh();
    };

    /*
    * Refresh a node in the primary classification by passing an identifier
    * @param string id (e.g n12345)
    */
    TREE.refresh_by_li = function(id) {
      var NODE = TREE.jsTree.get_node($(id));
      TREE.jsTree.refresh(NODE);
    };

    /*
    * Refresh a node in an alternate classification by passing an identifier
    * @param string id (e.g n12345)
    */
    TREE.refresh_by_li_alt = function(id) {
      var NODE = TREE.jsTree[$('.right_tree').attr("rel")].get_node($(id));
      TREE.jsTree[$('.right_tree').attr("rel")].refresh(NODE);
    };

    /*
    * Refresh a node by passing a node identifier and a tree identifier
    * @param string id (e.g n12345)
    * @param string tree_class (options are 'left_name_metadata' or other)
    */
    TREE.refresh_by_id = function(id,tree_class) {
      var parent = $(id).parent();
      var grandparent = parent.parent();
      if(tree_class == 'left_name_metadata') {
        var NODE = TREE.jsTree.get_node(grandparent);
        if(parent.attr('tagName') == 'UL') {
          TREE.jsTree.refresh(NODE);
        }
        else {
          TREE.jsTree.refresh();    
        }
      }
      else {
        var NODE = TREE.jsTree[$('.right_tree').attr('rel')].get_node(grandparent);
        if(parent.attr('tagName') == 'UL') {
          TREE.jsTree[$('.right_tree').attr('rel')].refresh(NODE);
        }
        else {
          TREE.jsTree[$('.right_tree').attr('rel')].refresh();  
        }           
      }
    };

    /*
    * Collapse all open branches in the primary classification
    */
    TREE.collapse = function() {
      TREE.jsTree.close_all();
    };

    /*
    * Collpase all open branches in an alternate classification
    */
    TREE.collapse_alt = function() {
      TREE.jsTree[$('.right_tree').attr("rel")].close_all();
    };

    /*
    * Deselect all in primary classification
    */
    TREE.deselect_all = function(TREE_OBJ) {
        var _this = this;
        $(TREE_OBJ).find("li:has(a.clicked)").each(function() {
          TREE.jsTree.deselect_branch(this);
        });
    };

    /*
    * Deselect all in alternate classification
    */
    TREE.deselect_all_alt = function(TREE_OBJ) {
        var _this = this;
        $(TREE_OBJ).find("li:has(a.clicked)").each(function() {
          TREE.jsTree[$('.right_tree').attr("rel")].deselect_branch(this);
        });
    };

    /*
    * Call back to retrieve the metadata object to populate the metadata panel
    * @param DOM element NODE
    * @param DOM element TREE_OBJ
    */
    TREE.get_metadata = function(NODE,TREE_OBJ) {
        var _this = this;
        var tid = this.get_id(NODE);
        var vid = TREE_OBJ.settings.vid;
        if(vid != primary_classification) {
          $('#metadata_taxon_tools').addClass('taxon_tools_right');
        }
        else {
          $('#metadata_taxon_tools').removeClass('taxon_tools_right');
        }
        $.get(Drupal.settings.classification_callback_base_url + "/get_metadata/" + vid + "/" + tid, {  }, function(data) { _this.build_metadata(data,vid); }, "json");
        $.get(Drupal.settings.classification_biblio_callback_base_url+"/get_citation/" + tid, { }, function(data) { 
            $('#edit-bibliographic-citation').val(data.citation);
        }, "json");
    };

    /*
    * Check if a node is a vernacular and show the vernacular options in the metadata panel
    * @param string elem (DOM form element for name relationship drop-down)
    */
    TREE.check_vern = function(ele) {
        if($(ele).val() == 'vernacular name' || $('select option:selected',$(ele).parent()).text() == 'vernacular name') {
            $('#classification_vernacular_wrapper').show();
        }
        else {
            $('#classification_vernacular_wrapper').hide();
        }
    };

    /*
    * Refresh a tree by passing an identifier
    * @param integer vid
    */
    TREE.refresh_tree = function(vid) {
      if(vid && vid > 0) {
        TREE.jsTree[vid].refresh(); 
      }
      else {
        TREE.jsTree.refresh();  
      }
    };

    /*
    * Callback so execute saving of metadata form
    */
    TREE.save_metadata = function() {
        var _this = this;
    
        //get all form elements, plus the hidden, original data and send it all back for comparison
        var metadata = $('#classification_name_metadata').data('metadata');

        var check = false;

        //first, check to make sure we have data in the jQuery element
        if(metadata.tid) {
          //push all the form data, including the original json in the DOM that was created with TREE.build_metadata (we need to compare server-side!)
          var alldata = [];
          alldata.push({name: 'original', value: JSON.stringify(metadata)});
          $('input,textarea,select,button', $('#classification_name_metadata')).each(function() {
            var n = this.name;
            var t = this.type;
            if ( !n || this.disabled || t == 'reset' ||
                (t == 'checkbox' || t == 'radio') && !this.checked ||
                (t == 'submit' || t == 'image' || t == 'button') && this.form.clicked != this ||
                this.tagName.toLowerCase() == 'select' && this.selectedIndex == -1)
                return;
            if (t == 'image' && this.form.clicked_x)
                return alldata.push(
                    {name: n+'_x', value: this.form.clicked_x},
                    {name: n+'_y', value: this.form.clicked_y}
                );
            if (t == 'select-multiple') {
                $('option:selected', this).each( function() {
                    alldata.push({name: n, value: this.value});
                });
                return;
            }
            alldata.push({name: n, value: this.value});
          });

          //Check to see if any form elements are required and contents are empty
          var required = true;
          var required_elements = '';
          $('input,textarea,select,button', $('#classification_name_metadata')).each(function() {
            if($(this).hasClass('classification_required') && !$(this).val()) {
              required_elements += '<li>' + $(this).prev().text().replace("*:","") + '</li>';
              required = false;
            }
          });

          //next, check all the relationship data in an async fashion and if all values acceptable, return var check=true
          if(!required) {
            var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
            message += '<div><strong>' + Drupal.t('The following fields are required:') + '</strong><ul class="classification_help_links">'+required_elements+'</ul></div>';
            message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
            message += '</div>';
            _this.show_message(message);
            $('#classification_ok').click(function() { 
              _this.hide_message();
              check = false;
            });
          }
          else {
            $.ajax({
              type: "POST",
              url: Drupal.settings.classification_callback_base_url + "/checkterms",
              data: alldata,
              dataType: "json",
              async: false,
              success: function(data) {
                if(data.vernacular_message) {
                    var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
                    message += '<div>'+data.vernacular_message+'</div>';
                    message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
                    message += '</div>';
                    message += '</div>';
                    _this.show_message(message);
                    $('#classification_ok').click(function() { 
                      _this.hide_message();
                      check = false;
                    });         
                }
                else if (data.missing && !data.vernacular_message) {
                    var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
                    message += '<div style="color:#FF0000;margin:3px">'+data.vernacular_message+'</div>';
                    message += data.message+'<br>';
                    message += data.data;
                    message += '<div style="margin-top:10px"><input id="edit-submit" type="submit" class="form-submit classification_missing_submit" value="Add Missing">';
                    message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
                    message += '</div>';
                    message += '</div>';
                    _this.show_message(message);
                    $('.classification_missing_submit').click(function() {
                      var addflags = $('#classification_warning_message').fastSerialize();
                      $.post(Drupal.settings.classification_callback_base_url + "/addflags", addflags, function(data) {
                        if(data.status) {
                          message = '<div>'+data.message+'</div>';
                          message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
                          $('#classification_warning_message').html(message);
                          $('#classification_ok').click(function() { _this.hide_message(); check = false; });
                        }
                        else {
                          message = '<div>'+data.message+'</div>';
                          message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
                          $('#classification_warning_message').html(message);
                          $('#classification_ok').click(function() { _this.hide_message(); check = false; });   
                        }
                      },"json");
                    });
                    $('#classification_cancel').click(function() { 
                      _this.hide_message();
                      check = false;
                    });
                }
                else {
                  check = true;
                }
              },
              fail: function(data) {
                var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
                message += Drupal.t('Sorry, something bad happened. Please try again.') + '<br>';
                message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
                message += '</div>';
                _this.show_message(message);
                $('#classification_ok').click(function() { _this.hide_message(); check = false; });         
              }          
            });
          }
        }
          
        if(check) {
          //push all the form data again and send it off
          var alldata = [];
          alldata.push({name: 'original', value: JSON.stringify(metadata)});
          $('input,textarea,select,button', $('#classification_name_metadata')).each(function() {
            var n = this.name;
            var t = this.type;
            if ( !n || this.disabled || t == 'reset' ||
                (t == 'checkbox' || t == 'radio') && !this.checked ||
                (t == 'submit' || t == 'image' || t == 'button') && this.form.clicked != this ||
                this.tagName.toLowerCase() == 'select' && this.selectedIndex == -1)
                return;
            if (t == 'image' && this.form.clicked_x)
                return alldata.push(
                    {name: n+'_x', value: this.form.clicked_x},
                    {name: n+'_y', value: this.form.clicked_y}
                );
            if (t == 'select-multiple') {
                $('option:selected', this).each( function() {
                    alldata.push({name: n, value: this.value});
                });
                return;
            }
            alldata.push({name: n, value: this.value});
          });

          $.post(Drupal.settings.classification_callback_base_url + "/update_metadata/", alldata, function(data) {
            _this.display_log(data);
          }, "json");
          $.post(Drupal.settings.classification_biblio_callback_base_url+"/update_citation/", { "tid" : metadata.tid, "citation" : $('#edit-bibliographic-citation').val()});
            
          if($('select',$('#classification_relation_wrapper')).val()) {
              var tree_class = $('#edit-submit').parent().parent().attr("class");
              _this.refresh_by_id('#n' + metadata.tid,tree_class);
          }
        
        }
        else {
          return false;
        }
    };

    /*
    * Callback to edit a node via inline editing
    * @param DOM element NODE
    * @param DOM element TREE_OBJ
    */
    TREE.edit_name = function(NODE,TREE_OBJ) {
      var _this = this;
      var vid = TREE_OBJ.settings.vid;
      var tid = this.get_id(NODE);
      var newname = this.get_content(NODE);
      $.post(Drupal.settings.classification_callback_base_url + "/edit_name/", { "vid" : vid, "tid" : tid, "value" : newname }, function(data) {
        if(data.status == 'duplicate') {
         var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
         message += data.message;
         message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Discard') + '">';
         message += '  <a id="classification_add_duplicate">' + Drupal.t('Add Anyway') + '</a>';
         message += '</div>'
         message += '</div>';
         _this.show_message(message);
         $('#classification_add_duplicate').click(function() {
            _this.hide_message();
            _this.get_metadata(NODE,TREE_OBJ);
          });
         $('#edit-delete').click(function() {
            TREE.jsTree.remove(NODE);
            _this.hide_message();
          });
        }
        else {
          _this.display_log(data);
          _this.get_metadata(NODE,TREE_OBJ);
        }
      }, "json");
    };

    /*
    * Callback to add a single node under a parent
    * @param DOM element NODE
    * @param DOM element REF_NODE
    * @param string TYPE (inside or before)
    * @param DOM element TREE_OBJ
    */
    TREE.add_name = function(NODE,REF_NODE,TYPE,TREE_OBJ) {
      var _this = this;
      switch(TYPE) {
        case 'inside':
          var parent_tid = (REF_NODE !== -1) ? REF_NODE.id.replace("n","") : 0;
        break;
        case 'before':
          var parent_tid = (REF_NODE !== -1) ? $(REF_NODE).parent().parent().attr("id").replace("n","") : 0;
        break;  
      }

      var vid = TREE_OBJ.settings.vid;
      $.post(Drupal.settings.classification_callback_base_url + "/add_name/", { "vid" : vid, "parent_tid" : parent_tid, "name" : $(NODE).text() }, function(data) { 
        $(NODE).attr('id', 'n' + data.tid);
        $('#edit-tid').val(data.tid);
        $('#edit-parent-tid').val('');
        $('#metadata_taxon_title').html('');
        $('#edit-relation-type').attr("disabled",false);
        $('#metadata_parent_name').html(data.parent_name);
        $('#edit-rank').val('');
        $('#edit-relation-id').val('');
        $('#edit-relation-type').val('');
        $('#edit-description').val('');
      }, "json");  
    };

    /*
    * User warning shown when action to delete a node in the primary classification is first exectuted
    * @param DOM element NODE
    * @param DOM element TREE_OBJ
    */
    TREE.delete_name_warning = function(NODE,TREE_OBJ) {
        if(!NODE || !TREE_OBJ) {
            NODE = TREE.jsTree.selected;
            TREE_OBJ = TREE.jsTree;
        }
        if(TREE_OBJ.container.find('#' + $(NODE).attr("id")).length == 0) return false;
        var status = TREE.check_status();
        if(status) {
          TREE.show_message('<div class="messages-throbber">' + Drupal.t('Checking for content associated with that name and all its children (if any)...') + '</div>');
          TREE.delete_name_data(NODE,TREE_OBJ);
        }
        return false;  
    };

    /*
    * User warning shown when action to delete a node in an alternate classification is first exectuted
    * @param DOM element NODE
    * @param DOM element TREE_OBJ
    */
    TREE.delete_name_warning_alt = function(NODE,TREE_OBJ) {
        if(!NODE || !TREE_OBJ) {
            NODE = TREE.jsTree[$('.right_tree').attr("rel")].selected;
            TREE_OBJ = TREE.jsTree[$('.right_tree').attr("rel")]; 
        }
        if(TREE_OBJ.container.find('#' + $(NODE).attr("id")).length == 0) return false;
        var status = TREE.check_status();
        if(status) {
            TREE.show_message('<div class="messages-throbber">' + Drupal.t('Checking for content associated with that name and all its children (if any)...') + '</div>');
            TREE.delete_name_data_alt(NODE,TREE_OBJ);
        }
        return false;      
    };

    /*
    * Delete callback for the primary classification after a user confirms the delete pop-up
    * Action is to first look for content associated with all children
    * @param DOM element NODE
    * @param DOM element TREE_OBJ
    */
    TREE.delete_name_data = function(NODE,TREE_OBJ) {
        var _this = this;
        var tid = this.get_id(NODE);
        var vid = TREE_OBJ.settings.vid;
        var name = this.get_content(NODE);
        var $message = $('#classification-message');
        $.ajax({
          type: "GET",
          url: Drupal.settings.classification_callback_base_url + "/delete_name_data/" + vid + "/" + tid,
          dataType: "json",
          success: function(data) {
              if(!data.names) {
                var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
                message += data.message;
                message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Delete"') + '>';
                message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
                message += '</div></div>';
                $message.html(message);
                $('#classification_cancel').click(function() {_this.hide_message();return false;});
                $('#edit-delete').click(function() {
                  TREE_OBJ.remove(NODE);
                });
              }
              else {
                var message = '<div id="classification_warning_message" class="classification-warning messages error" style="margin:0px auto">';
                message += data.message;
                message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Delete') + '">';
                message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
                message += '</div></div>';
                $message.html(message);
                $('#classification_cancel').click(function() {_this.hide_message();return false;});
                $('#edit-delete').click(function() {
                  TREE_OBJ.remove(NODE);
                });
                $('#classification_warning_message').append('<div id="classification_affected_names"><strong>' + Drupal.t('Affected Taxa:') + '</strong><div id="classification_delete_affected_wrapper"><div id="classification_delete_affected"></div></div></div>');
                $.each(data.names,function(i,namedata) {
                    var name = '<a id="affected_name_' + namedata.tid + '" class="classification_delete_affected">' + namedata.name + '</a>';
                    $('#classification_delete_affected').append(name + '<br>');
                    $('#affected_name_' + namedata.tid).click(function() { window.open(Drupal.settings.basePath + 'pages/' + namedata.tid); });
                });     
              } 
          },
          error: function(data) {
              var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
              message += Drupal.t('Sorry, there were too many names to check for content. Please select children of ' + name + ' and refine your deletion.') + '<br>';
              message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
              message += '</div>';
              $message.html(message);
              $('#classification_ok').click(function() {_this.hide_message(); return false;});
          }
        });
    };

    /*
    * Delete callback for an alternate classification after a user confirms the delete pop-up
    * Action is to look for content associated with all children
    * @param DOM element NODE
    * @param DOM element TREE_OBJ
    */
    TREE.delete_name_data_alt = function(NODE,TREE_OBJ) {
        var _this = this;
        var tid = this.get_id(NODE);
        var vid = TREE_OBJ.settings.vid;
        var name = this.get_content(NODE);
        var $message = $('#classification-message');
        $.ajax({
          type: "GET",
          url: Drupal.settings.classification_callback_base_url + "/delete_name_data/" + vid + "/" + tid,
          dataType: "json",
          success: function(data) {
              if(!data.names) {
                var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
                message += data.message;
                message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Delete') + '">';
                message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
                message += '</div></div>';
                $message.html(message);
                $('#classification_cancel').click(function() {_this.hide_message();return false;});
                $('#edit-delete').click(function() {TREE_OBJ.remove();});
              }
              else {
                var message = '<div id="classification_warning_message" class="classification-warning messages error" style="margin:0px auto">';
                message += data.message;
                message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Delete') + '">';
                message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
                message += '</div></div>';
                $message.html(message);
                $('#classification_cancel').click(function() {_this.hide_message();return false;});
                $('#edit-delete').click(function() {TREE_OBJ.remove();});
                $('#classification_warning_message').append('<div id="classification_affected_names"><strong>' + Drupal.t('Affected Taxa:') + '</strong><div id="classification_delete_affected_wrapper"><div id="classification_delete_affected"></div></div></div>');
                $.each(data.names,function(i,namedata) {
                    var name = '<a id="affected_name_' + namedata.tid + '" class="classification_delete_affected">' + namedata.name + '</a>';
                    $('#classification_delete_affected').append(name + '<br>');
                    $('#affected_name_' + namedata.tid).click(function() { window.open(Drupal.settings.basePath + 'pages/' + namedata.tid); });
                });     
              } 
          },
          error: function(data) {
              var message = '<div id="classification_warning_message" class="classification-warning messages warning" style="margin:0px auto">';
              message += Drupal.t('Sorry, there were too many names to check for content. Please select children of !name and refine your deletion.', {'!name' : name}) + '<br>';
              message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
              message += '</div>';
              $message.html(message);
              $('#classification_ok').click(function() {_this.hide_message(); return false;});
          }
        });
    };

    /*
    * Callback to actually delete a node
    * @param DOM element NODE
    * @param integer vid
    */
    TREE.delete_name = function(NODE,vid) {
        var _this = this;
        var tid = this.get_id(NODE);
        var name = this.get_content(NODE);
        var message = '<div class="messages-throbber">' + Drupal.t('Deleting !name and its children (if any)...', {'!name' : name}) + '</div>';
        $('#classification-message').html(message);
        $.post(Drupal.settings.classification_callback_base_url + "/delete_name/", {"tid" : tid, "vid" : vid, "name" : name }, function(data) { 
          _this.success_delete(data);
          _this.display_log(data);
        }, "json");
        $('#edit-vern-lang-wrapper').hide();
        $('#edit-tid').val('');
        $('#edit-parent-tid').val('');
        $('#metadata_taxon_title').html('');
        $('#edit-rank').val('');
        $('#edit-relation-id').val('');
        $('#edit-relation-type').val('');
        $('#edit-description').val('');
        return true;
    };

    /*
    * Refresh the deleted names classification upon successful deletion of a node elsewhere
    */
    TREE.success_delete = function(data) {
          var _this = this;
          if(TREE.jsTree[data.delete_vid]) {
            TREE.refresh_tree(data.delete_vid);
          }
          _this.hide_message();
    };

    /*
    * Hide the pop-up message
    */
    TREE.hide_message = function() {
      $('#classification-message').hide();
      $('#classification-overlay').hide();
    };

    /*
    * Callback to move a node
    * @param DOM element NODE
    * @param DOM element REF_NODE
    * @param string TYPE (e.g. after, before, inside)
    * @param DOM element TREE_OBJ
    */
    TREE.move_name = function(NODE,REF_NODE,TYPE,TREE_OBJ) {
        var _this = this;
        var orig = this.get_id(NODE);
        var dest = this.get_id(REF_NODE);
        var vid = TREE_OBJ.settings.vid;

        switch(TYPE) {
            case "after":
               //must use parent of REF_NODE.id as new parent, so we need another callback to get it & then perform the move
               $.post(Drupal.settings.classification_callback_base_url + "/get_parent/", {"child" : orig, "child_parent" : dest}, function(data) {_this.move_name_2(data,vid);}, "json");
             break;
           case "before":
             //must use parent of REF_NODE.id as new parent, so we need another callback to get it and then perform the move
             $.post(Drupal.settings.classification_callback_base_url + "/get_parent/", {"child" : orig, "child_parent" : dest}, function(data) {_this.move_name_2(data,vid);}, "json");
             break;
           case "inside":
             //may use REF_NODE.id as new parent
             $.post(Drupal.settings.classification_callback_base_url + "/move_name/", {"child" : orig, "new_parent" : dest, "vid" : vid }, function(data) {
               _this.display_log(data);
             }, "json");
             break;
        }
    };

    /*
    * Helper method to move a name (see TREE.move_name)
    * @param object content
    * @param integer vid
    */
    TREE.move_name_2 = function(content,vid) {
        var _this = this;
        var orig = content.child;
        var dest = content.dest;
        $.post(Drupal.settings.classification_callback_base_url + "/move_name/", {"child" : orig, "new_parent" : dest, "vid" : vid }, function(data) {
          _this.display_log(data);
        }, "json");
    };

    /*
    * Callback to copy a node
    * @param DOM element NODE
    * @param DOM element REF_NODE
    * @param string TYPE (e.g. after, before, inside)
    * @param DOM element TREE_OBJ
    */
    TREE.copy_name = function(NODE,REF_NODE,TYPE,TREE_OBJ) {
      var _this = this;
      var orig = this.get_id(NODE);
      var dest = this.get_id(REF_NODE);
      var vid = TREE_OBJ.settings.vid;

      switch(TYPE) {
        case "after":
           //must use parent of REF_NODE.id as new parent, so we need another callback to get it & then perform the move
           $.post(Drupal.settings.classification_callback_base_url + "/get_parent/", {"child" : orig, "child_parent" : dest}, function(data) {_this.copy_name_2(data,vid);}, "json");
         break;
       case "before":
         //must use parent of REF_NODE.id as new parent, so we need another callback to get it and then perform the move
         $.post(Drupal.settings.classification_callback_base_url + "/get_parent/", {"child" : orig, "child_parent" : dest}, function(data) {_this.copy_name_2(data,vid);}, "json");
         break;
       case "inside":
         //may use REF_NODE.id as new parent
         $.post(Drupal.settings.classification_callback_base_url + "/copy_name/", {"child" : orig, "new_parent" : dest, "vid" : vid }, function(data) {
           _this.display_log(data);
           if(dest==0) { TREE.jsTree.refresh() } else { TREE.refresh_by_li('#n' + dest) } 
         }, "json");
         break; 
      }
    };

    /*
    * Helper method to copy a node (see TREE.copy_name)
    * @param object content
    * @param integer vid
    */
    TREE.copy_name_2 = function(content,vid) {
        var _this = this;
        var orig = content.child;
        var dest = content.dest;
        $.post(Drupal.settings.classification_callback_base_url + "/copy_name/", {"child" : orig, "new_parent" : dest, "vid" : vid }, function(data) {
          _this.display_log(data);
          if(dest==0) { TREE.jsTree.refresh() } else { TREE.refresh_by_li('#n' + dest) } 
        }, "json");    
    };

    // Static variables to set/unset status of metadata panels
    TREE.toggled_left = false;
    TREE.toggled_right = true;

    /*
    * Show the metadata panel
    * @param string tree (left or right)
    */
    TREE.show_metadata = function(tree) {
        var $metadata = $('#classification_name_metadata');
        switch(tree) {
          case 'left':
            if(!TREE.jsTree.selected) {
              return false;
            }
            $('.right_tree').css('z-index','1');
            $('.left_tree').css('z-index','2');
            if($(TREE.jsTree.selected).text() == $('#metadata_taxon_title').text()) {
                if(!this.toggled_left) {
                  $('#metadata_taxon_link span').show();
                  $('#classification_metadata_wrapper span').removeClass('right').addClass('left');
                  $metadata.removeClass('right_name_metadata').addClass('left_name_metadata');
                  $('.left_tree').append($metadata);
                  $metadata.slideRight('fast').show();
                  $(".classification_show_metadata span.left").text(Drupal.t('Hide'));
                  this.toggled_left = true;
                }
                else {
                  $(".classification_show_metadata span.left").text(Drupal.t('Show'));
                  this.hide_metadata(tree);
                  this.toggled_left = false;
                }           
            }
            else {
              this.get_metadata(TREE.jsTree.selected,TREE.jsTree);
              
              if(!this.toggled_left) {
                $('#metadata_taxon_link span').show();
                $('#classification_metadata_wrapper span').removeClass('right').addClass('left');
                $metadata.removeClass('right_name_metadata').addClass('left_name_metadata');
                $('.left_tree').append($metadata);
                $metadata.slideRight('fast').show();
                $(".classification_show_metadata span.left").text(Drupal.t('Hide'));
                this.toggled_left = true;
              }
              else {
                $(".classification_show_metadata span.left").text(Drupal.t('Show'));
                this.hide_metadata(tree);
                this.toggled_left = false;
              }
            }
          break;
        
          case 'right':
            if(!TREE.jsTree[$('.right_tree').attr('rel')] || !TREE.jsTree[$('.right_tree').attr('rel')].selected) {
              return false;
            }
            $('.right_tree').css('z-index','2');
            $('.left_tree').css('z-index','1');
            if(TREE.jsTree[$('.right_tree').attr('rel')] && $(TREE.jsTree[$('.right_tree').attr('rel')].selected).text() == $('#metadata_taxon_title').text()) {
              if(!this.toggled_right) {
                $('#metadata_taxon_link span').hide();
                $('#classification_metadata_wrapper span').removeClass('left').addClass('right');
                $metadata.removeClass('left_name_metadata').addClass('right_name_metadata');
                $('.right_tree').append($metadata);
                $metadata.slideRight('fast').show();
                $(".classification_show_metadata_alt span.right").text(Drupal.t('Hide'));
                this.toggled_right = true;
              }
              else {
                $(".classification_show_metadata_alt span.right").text(Drupal.t('Show'));
                this.hide_metadata(tree);
                this.toggled_right = false;
              }
            }
            else if (TREE.jsTree[$('.right_tree').attr('rel')]) {
              this.get_metadata(TREE.jsTree[$('.right_tree').attr('rel')].selected,TREE.jsTree[$('.right_tree').attr('rel')]);
              if(!this.toggled_right) {
                $('#metadata_taxon_link span').hide();
                $('#classification_metadata_wrapper span').removeClass('left').addClass('right');
                $metadata.removeClass('left_name_metadata').addClass('right_name_metadata');
                $('.right_tree').append($metadata);
                $metadata.slideRight('fast').show();
                $(".classification_show_metadata_alt span.right").text(Drupal.t('Hide'));
                this.toggled_right = true;
              }
              else {
                $(".classification_show_metadata_alt span.right").text(Drupal.t('Show'));
                this.hide_metadata(tree);
                this.toggled_right = false;
              }
            }
          break;
        }
    };

    /*
    * Hide the metadata panel
    * @param string tree (left or right)
    */
    TREE.hide_metadata = function(tree) {
        var $metadata = $('#classification_name_metadata');
        switch(tree) {
          case 'left':
            $(".classification_show_metadata span.left").text(Drupal.t('Show'));
          break;
        
          case 'right':
            $(".classification_show_metadata_alt span.right").text(Drupal.t('Show'));
          break;
        }
        $metadata.slideLeft('fast').hide();
    };

    /*
    * Push a notification into the scrolling log messages
    * @param object content
    */
    TREE.display_log = function(content) {
         var message_type = "";
         var message_color = "#00FF00";
         switch (content.status) {
              case "edited":
                message_type = "status";
                break;
              case "added":
                $('#n' + content.tid_parent + ' li:first-child').attr('id','n' + content.tid_child);
                message_type = "status";
                break;
              case "bulkadded":
                message_type = "status";
                break;
              case "deleted":
                message_type = "error";
                message_color = "#FF0000";
                break;
              case "moved":
                message_type = "status";
                break;
              case "copied":
                message_type = "status";
                break
              case "updated":
                message_type = "status";
                break;
              case "failed":
                message_type = "error";
                message_color = "#FF0000";
                break;
              case "warning":
                message_type = "warning";
                message_color = "#FFFF00";
                break;
              default:
                message_type = "status";
         }
         if(content.status == "failed") {
         }
         else if (classification_logging == 0) {
           return;
         }
         else {
            $('.classification-logs').prepend("<div class=\"classification-logs-message\"><span>" + content.message + "</span></div>");
            $('.classification-logs div:first span').effect("highlight", { color : message_color }, 2000);
         }
    };

    /*
    * Show a pop-up message prior to user executing action
    * @param string message (html content for message)
    */
    TREE.show_message = function(message) {
      $('body').append('<div id="classification-overlay"></div><div id="classification-message"></div>');
      var arrPageSizes = ___getPageSize();
      $('#classification-overlay').css({
        backgroundColor: 'black',
        opacity: 0.66,
        width: arrPageSizes[0],
        height: arrPageSizes[1]
      });
      var arrPageScroll = ___getPageScroll();
      $('#classification-message').css({
        top: 150,
        left: arrPageScroll[0],
        position: 'fixed',
        zIndex: 1001,
        margin: '0px auto',
        width: '100%'
      });
      $('#classification-overlay').show();
      $('#classification-message').html(message).show();
    };

    /*
    * Worker method to update content of metadata panel
    * @param object content
    * @param integer vid
    */
    TREE.build_metadata = function(content,vid) {
         var _this = this;
        
         //hide the vernacular selection at first
         $('#classification_vernacular_wrapper').hide();
    
         var $metadata = $('#classification_name_metadata');
    
         var data = content.data;
         //clear out any existing jQuery.data stored in places then put it all back so we can retrieve it later
         $metadata.removeData('metadata');
         $('.classification_ranks').removeData('rank');
         $('.classification_vernaculars').removeData('vernacular');

         $metadata.data('metadata',data);

         var tid = data.tid;
         var name = data.name;
         var description = data.description;

         var parent_tid = data.parent_tid;
         var parent_name = data.parent_name;
         var taxon_link = Drupal.settings.basePath + 'pages/' + tid;
         var content = data.content;

         var relations = data.relations;
        
         for(i=0;i<relations.length;i++) {
           var dom_name = relations[i].dom_name;
           var select = $("select[name="+dom_name+"]");
           var input = $("input[name="+dom_name+"]");
           if(relations[i].type != 'vernacular_language_codes') {
           switch(relations[i].type) {
             case 'taxonomic_ranks':
               //accommodate both textfield or select possibilities
               if(relations[i].tid && relations[i].value) {
                 if(select.length) {
                   $('.classification_ranks').data("rank",relations[i].tid);
                 }
                 else {
                   $('.classification_ranks').data("rank",relations[i].value);
                 }
                 select.val(relations[i].tid);
                 input.val(relations[i].value);
               }
               else {
                 select.val('');
                 input.val('');
               }
             break;
             case 'taxonomic_relationships':

               if(parent_tid) {
                 input.attr("disabled",false);
                 select.attr("disabled",false);
               }
               else {
                 input.attr("disabled",true);
                 select.attr("disabled",true);
               }
            
               //accommodate both textfield or select possibilities
               if(relations[i].tid && relations[i].value) {
                 select.val(relations[i].tid);
                 input.val(relations[i].value);
                 $('#metadata_taxon_link span').hide();
                 var color = $("#classification_name_metadata").hasClass("left_name_metadata") ? "#CCFFCC" : "#E6E6E6";
                 $('.classification_ranks').css("background-color",color).attr("disabled",true);
               }
               else {
                 $('#metadata_taxon_link span').show();
                 $('.classification_ranks').css("background-color","").removeAttr("disabled");
                 select.val('');
                 input.val('');
               }
               
               switch(relations[i].value) {
        
                 case '':
                   name = "<span style='color:black'>" + name + "</span>";
                 break;
        
                 case 'vernacular name':
                   $('#classification_vernacular_wrapper').show();
                   name = "<span style='color:#228B22'>" + name + "</span>";
                 break;
        
                 case 'basionym':
                   name = "<span style='font-weight:bold'>" + name + "</span>";
                 break;
        
                 default:
                   name = "<span style='color:#FF0000'>" + name + "</span>";
               } 

             break;
             case 'names_to_vernacular_codes':
               //accommodate both textfield or select possibilities
               if(relations[i].tid && relations[i].value) {
                 vern_value = relations[i].value.split(",");
                 vern_value = vern_value[1];
                 select.val(relations[i].tid);
                 input.val(vern_value);
                 if(select.length) {
                   $('.classification_vernaculars').data("vernacular",relations[i].tid);
                 }
                 else {
                   $('.classification_vernaculars').data("vernacular",relations[i].value);
                 }
               }
               else {
                 select.val('');
                 input.val('');
               }
             break;
             default:
               //accommodate both textfield or select possibilities
               if(relations[i].tid && relations[i].value) {
                 select.val(relations[i].tid);
                 input.val(relations[i].value);
               }
               else {
                 select.val('');
                 input.val('');
               }
             }
           }
         }

         $('#metadata_taxon_title').html(name);
         $('a.metadata_taxon_link_href').attr('href',taxon_link);
         
         $('#edit-description').val(description);

         if(parent_name) {
           $('#metadata_parent_name').html(parent_name);
         }
         else {
            $('#metadata_parent_name').html("<i>(no parent specified)</i>");
         }

         if(content) {
           $('#metadata_taxon_content span').show();
         }
         else {
           $('#metadata_taxon_content span').hide();
         }
    
  };

  /*
  * Callback to entirely delete an alternate classification
  */
  TREE.delete_classification = function() {
        var _this = this;
        var classification_name = $("ul.classification_topnav li a:first").text();
        var classification_vid = $('.right_tree').attr("rel");
        var message = '<div id="classification_warning_message" class="classification-warning messages error" style="margin:0px auto">';
        message += Drupal.t('Are you sure you want to permanently delete %classification with no chance of recovery?', {'%classification' : classification_name});
        message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Delete') + '">';
        message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
        message += '</div></div>';
        this.show_message(message);
    
        $('#classification_cancel').click(function() {
          _this.hide_message();
        });
    
        $('#edit-delete').click(function() {
            message = '<div class="messages-throbber">' + Drupal.t('Deleting %classification...', {'%classification' : classification_name}) + '</div>';
            $('#classification-message').html(message);
    
            $.post(Drupal.settings.classification_callback_base_url + "/delete_classification/", {'vid' : classification_vid}, function(data) {
              switch(data.status) {
                case 'ok':
                  var status = 'status';
                break;
            
                case 'failed':
                  var status = 'error';
                break;
              }
              message = '<div class="classification-warning messages ' + status + '" style="margin:0px auto">' + data.message;
              message += (status == 'error') ? '<div style="margin-top:10px"><input id="classification_ok" type="submit" class="form-submit" value="' + Drupal.t('OK') + '"></div>' : '';
              message += '</div>';
              $('#classification-message').html(message);
              if(status != 'error') {
                $('#classification-message').delay(2000,function() {
                  _this.hide_message();
                  window.location.reload(true);
                });       
              }
              $('#classification_ok').click(function() {
                _this.hide_message();
                window.location.reload(true);
              });
            }, 'json');
        });
  };

  //deselect nodes when click
  $('#classification_tree_' + primary_classification).live('click', function() {
    TREE.deselect_all(this);
  });

  //deselect nodes in alternate trees when click
  $('#classification_tree_alternate').live('click', function() {
    TREE.deselect_all_alt(this);
  });

  //highlight the classification drop-down when hovered
  $("ul.classification_topnav li:first").hover(
    function() {
        $(this).animate({backgroundColor : "#E6E6E6"});
    },
    function() {
        $(this).animate({backgroundColor : "#FFFFFF"});
  });

  //show the metadata panel when a tid is passed in URL
  if(tid && !classification_open) {
    TREE.show_metadata('left');
  }

  //Expand an alternate tree if vid in url
  //show the alternate tree when a vid is passed in URL
  if(classification_open) {
      var classification_name = $('ul.classification_subnav li[rel=' + classification_open + ']').text();
      $("ul.classification_topnav li a").eq(0).text(classification_name);
      $('.right_tree').attr("rel",classification_open);
      $("#edit-classification-search2-autocomplete").val(Drupal.settings.alt_search + $('.right_tree').attr("rel"));
      var uri = $("#edit-classification-search2-autocomplete").val();
      //adjust the autocomplete search.
      //Because autocomplete.js uses a Drupal.ABCD object, 
      //the best we can do is ensure the same uri doesn't get hit multiple times when swapping classifications
      //so we dump the uris into our own abcd array for checking
      if(!abcd[uri]) {
        $("#edit-classification-search2-autocomplete").removeClass("autocomplete-processed");
        Drupal.behaviors.autocomplete();
        abcd[uri] = uri;
      }
    
      if(classification_open == deleted_names || classification_open == lost_found) {
        $('.classification_alt_del').hide();
        $('#deleted_names_message').show();
        if(classification_open == deleted_names) {
          $('#classification_purge_deleted').show();
        }
        else {
          $('#classification_purge_deleted').hide();
        }
        $('.classification_secondary').find("a").each(function() {
          var reltag = $(this).attr("rel");
          if(reltag && reltag !== 'refresh_alt' && reltag !== 'cut_alt' && reltag !== 'collapse_alt') {
            $(this).hide();
          }
          if(reltag && classification_open == lost_found && reltag == 'delete_name_warning_alt') {
            $(this).show();
          }
        });
      }
      else {
        $('.classification_alt_del').show();
        $('.classification_secondary').find("a").each(function() {
          $(this).show();
        });
        $('#deleted_names_message').hide();
      }
    
      TREE.get_tree(classification_open);
  }

  //keyboard shortcuts depenedent on OS
  if($.os.name == 'mac') {
    var new_key = 'command+n';
    var cut_key = 'command+x';
    var copy_key = 'command+c';
    var paste_key = 'command+v';
  }
  else {
    var new_key = 'ctrl+n';
    var cut_key = 'ctrl+x';
    var copy_key = 'ctrl+c';
    var paste_key = 'ctrl+v';   
  }
    
  $(document)
    .bind('keydown', {combi:'up', disableInInput: true} , function() { TREE.jsTree.get_prev(); return false; })
    .bind('keydown', {combi:'down', disableInInput: true} , function() { TREE.jsTree.get_next(); return false; })
    .bind('keydown', {combi:'left', disableInInput: true} , function() { TREE.jsTree.get_left(); return false; })
    .bind('keydown', {combi:'right', disableInInput: true} , function() { TREE.jsTree.get_right(); return false; })
    .bind('keydown', {combi:'return', disableInInput: true} , function() { if(TREE.jsTree.hovered) TREE.jsTree.select_branch(TREE.jsTree.hovered); return false; })
    .bind('keydown', {combi:'f2', disableInInput: true} , function() { if(TREE.jsTree.hovered) TREE.rename(); return false; })
    .bind('keydown', {combi:'del', disableInInput: true} , function() { if(TREE.jsTree.selected) TREE.delete_name_warning(); return false; })
    .bind('keydown', {combi:new_key, disableInInput: true} , function() { if(TREE.jsTree.selected) TREE.create_child();  return false; })
    .bind('keydown', {combi:cut_key, disableInInput: true} , function() { TREE.jsTree.cut(); return false; })
    .bind('keydown', {combi:paste_key, disableInInput: true} , function() { TREE.jsTree.paste(); return false; });

  //interface hooks
   $(".tree_actions_bar")
    .find("a")
    .bind("click", function () {
      try { TREE[$(this).attr("rel")](); } catch(err) { }
      this.blur();
    })
   .end().end();

  //tooltips jQuery extension setting
  $('.classification-tooltip').tipsy({gravity: 's'});

  $('.classification_edit_search_button').click(function() {
    var search = $(this).prev().val();
    $.tree_reference('classification_tree_' + primary_classification).search(search);
  });

  $('.alt_search_button').click(function() {
    var search = $(this).prev().val();
    var tree_id = $('.right_tree').attr("rel");
    $.tree_reference('classification_tree_' + tree_id).search(search);
  });

    var menu = $('#metadata_actions');
      $(menu).find('A').mouseover( function() {
      $(menu).find('LI.hover').removeClass('hover');
      $(this).parent().addClass('hover');
      }).mouseout( function() {
        $(menu).find('LI.hover').removeClass('hover');
      });
    $('#edit-root-save').appendTo('#edit-root-wrapper');
    //set the interface

    $("ul.classification_topnav li a:first").css('font-size','16px');
    
    $("ul.classification_topnav li").eq(0).click(function() {
        $(this).find("ul.classification_subnav").css('z-index','6').slideDown('fast').show();
        $("ul.classification_subnav").hover(function() { }, function(){
          $(this).slideUp('slow');
        });
    });
    
    $("ul.classification_subnav li").click(function() {
      $("ul.classification_topnav li a").eq(0).text($(this).text());
      $('.right_tree').attr("rel",$(this).attr("rel"));
      $("#edit-classification-search2-autocomplete").val(Drupal.settings.alt_search + $('.right_tree').attr("rel"));
      var uri = $("#edit-classification-search2-autocomplete").val();
      //adjust the autocomplete search.
      //Because autocomplete.js uses a Drupal.ABCD object, 
      //the best we can do is ensure the same uri doesn't get hit multiple times when swapping classifications
      //so we dump the uris into our own abcd array for checking
      if(!abcd[uri]) {
        $("#edit-classification-search2-autocomplete").removeClass("autocomplete-processed");
        Drupal.behaviors.autocomplete();
        abcd[uri] = uri;
      }
      if($(this).attr("rel") == deleted_names || $(this).attr("rel") == lost_found) {
        var parent_rel = $(this).attr("rel");
        $('.classification_alt_del').hide();
        $('#deleted_names_message').show();
        if($(this).attr("rel") == deleted_names) {
          $('#classification_purge_deleted').show();
        }
        else {
          $('#classification_purge_deleted').hide();
        }
        $('.classification_secondary').find("a").each(function() {
          var reltag = $(this).attr("rel");
          if(reltag && reltag !== 'refresh_alt' && reltag !== 'cut_alt' && reltag !== 'collapse_alt') {
            $(this).hide();
          }
          if(reltag && parent_rel == lost_found && reltag == 'delete_name_warning_alt') {
            $(this).show();
          }
        });
      }
      else {
        $('.classification_alt_del').show();
        $('.classification_secondary').find("a").each(function() {
          $(this).show();
        });
        $('#deleted_names_message').hide();
      }
      TREE.get_tree($(this).attr("rel"));
    });

    $('#classification-edit-form').click(function(e) {
      var e = $(e.target);
      if(!e.parents().hasClass('classification_topnav')) {
        $("ul.classification_subnav").slideUp('slow');  
      }
    });

    $('#classification_metadata_wrapper span').click(function() {
      TREE.toggled_left = false;
      TREE.toggled_right = true;
      var metadata_class = $(this).parent().parent().attr("class");
      if(metadata_class == 'left_name_metadata') {
        TREE.hide_metadata('left');
      }
      else {
        TREE.hide_metadata('right');
      }
    });

    $('#classification_purge_deleted').click(function() {
        var message = '<div id="classification_warning_message" class="classification-warning messages error" style="margin:0px auto">';
        message += Drupal.t('Are you sure you want to permanently delete all names from your <strong>Deleted Names</strong>? Names with content will be preserved, but they will lose their hierarchy.');
        message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Delete') + '">';
        message += '  <a id="classification_cancel">' + Drupal.t('Cancel') + '</a>';
        message += '</div></div>';
        TREE.show_message(message);
        $('#classification_cancel').click(function() {TREE.hide_message();return false;});
        $('#edit-delete').click(function() {
          var newmessage = '<div class="messages-throbber">' + Drupal.t('Clearing all deleted names...') + '</div>';
          $('#classification-message').html(newmessage);
          $.post(Drupal.settings.classification_callback_base_url + "/purge_deletions/", { "vid" : deleted_names }, function(data) {
            TREE.jsTree[deleted_names].refresh();
            TREE.hide_message();
            }, "json");
        });
    });
    
    $('.classification_relations').change(function() {
      var color = $("#classification_name_metadata").hasClass("left_name_metadata") ? "#CCFFCC" : "#F0F0EE";
      if($(this).val()){
        $(".classification_ranks").attr("disabled",true).css("background-color",color).val("");
      }
      else {
        var rank = $(".classification_ranks").data("rank");
        $('.classification_ranks').removeAttr("disabled").css("background-color","").val(rank); 
      }
    });

    /*
    * Periodic polling by all users (if enabled) such that actions propogate to all other users' screens
    */
    if(classification_logging) {

      jug.subscribe(Drupal.settings.juggernaut_channel, function(data) {
            if(data.status && data.message && data.sid != Drupal.settings.sid) {
                TREE.display_log(data);
            }
            if(data.actions && data.actions.length > 0 && data.sid != Drupal.settings.sid) {
                
                //chunk data actions array into pieces, then loop through pairs of chunks
                var actionsChunk = chunk(data.actions, 2);
                
                for(var i=0; i<actionsChunk.length; i++) {
                    
                    var child = actionsChunk[i][0].tid;
                    var parent = actionsChunk[i][0].parent;
                    var vid = actionsChunk[i][0].vid;
                    if(child && !parent) parent = "0"; //necessary because 0 root is ev'l as undefined

                    var new_child = (actionsChunk[i][1] && actionsChunk[i][1].tid) ? actionsChunk[i][1].tid : "";
                    var new_parent = (actionsChunk[i][1] && actionsChunk[i][1].parent) ? actionsChunk[i][1].parent : "";
                    var new_vid = (actionsChunk[i][1] && actionsChunk[i][1].vid) ? actionsChunk[i][1].vid : "";
                    if(new_child && !new_parent) new_parent = "0";

                    //alert change that's about to take place
                    $('#classification_tree_' + primary_classification + ' #n'+child + ' a:first').effect("highlight", { color : "#FF0000" }, 2000);

                    if(new_vid && new_vid == deleted_names || new_vid == lost_found) {
                        $('#classification_tree_' + new_vid + ' #n'+new_child + ' a:first').effect("highlight", { color : "#FF0000" }, 2000);
                    }

                    //close the metadata panel if an affected term is currently being viewed
                    var metadata = $('#classification_name_metadata').data('metadata');
                    if(metadata && metadata.tid && (metadata.tid == child || metadata.tid == new_child)) {
                        TREE.hide_metadata('left');
                        TREE.hide_metadata('right');
                    }

                    //refresh the parents, but only if necessary
                    if($('#classification_tree_' + primary_classification + ' #n'+parent).length > 0) {
                        //first we need to determine if the node can still be expanded
                        var children = TREE.count_children(primary_classification, parent);
                        if(children && $('#classification_tree_' + primary_classification + ' #n'+parent).hasClass("open")) {
                            TREE.refresh_by_li('#n'+parent);
                        }
                        else if(children && $('#classification_tree_' + primary_classification + ' #n'+parent).hasClass("closed")) {
                            if(new_vid == deleted_names) {
                                $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#00FF00" }, 2000);
                            }
                            else if(!new_vid) {
                                $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#FF0000" }, 2000);
                            }
                            else if(vid != new_vid) {
                                $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#00FF00" }, 2000);
                            }
                        }
                        else if(children && $('#classification_tree_' + primary_classification + ' #n'+parent).hasClass("leaf")) {
                            $('#classification_tree_' + primary_classification + ' #n'+parent).removeClass("leaf").addClass("closed");
                            $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#00FF00" }, 2000);
                        }
                        else if(!children && $('#classification_tree_' + primary_classification + ' #n'+parent).hasClass("closed")) {
                            $('#classification_tree_' + primary_classification + ' #n'+parent).removeClass("closed").addClass("leaf");
                            $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#FF0000" }, 2000);
                        }
                        else if(!children && $('#classification_tree_' + primary_classification + ' #n'+parent).hasClass("open")) {
                            $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#FF0000" }, 2000);
                            //refresh the grandparent
                            var grandparent = $('#classification_tree_' + primary_classification + ' #n'+parent).parent().parent().attr("id");
                            TREE.refresh_by_li('#'+grandparent);
                        }
                        else if(!children && $('#classification_tree_' + primary_classification + ' #n'+parent).hasClass("leaf")) {
                            $('#classification_tree_' + primary_classification + ' #n'+parent).removeClass("leaf").addClass("closed");
                            $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#00FF00" }, 2000);
                        }
                    }

                    else if(!child) {
                        //purge deletions
                    }
                    else if(!parent || parent == 0){
                        TREE.refresh_tree();
                    }
                    else {}

                    if($('#classification_tree_' + primary_classification + ' #n'+new_parent).length > 0) {
                        var children = TREE.count_children(primary_classification, new_parent);
                        if(children && $('#classification_tree_' + primary_classification + ' #n'+new_parent).hasClass("open")) {
                            TREE.refresh_by_li('#n'+new_parent);
                        }
                        if(children && $('#classification_tree_' + primary_classification + ' #n'+new_parent).hasClass("leaf")) {
                            $('#classification_tree_' + primary_classification + ' #n'+new_parent).removeClass("leaf").addClass("closed");
                        }
                        if(children && $('#classification_tree_' + primary_classification + ' #n'+new_parent).hasClass("closed")) {
                            $('#classification_tree_' + primary_classification + ' #n'+new_parent + ' a:first').effect("highlight", { color : "#00FF00" }, 2000);
                            if(vid == new_vid) {
                                $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#FF0000" }, 2000);
                            }
                        }
                        if(!children && $('#classification_tree_' + primary_classification + ' #n'+new_parent).hasClass("open")) {
                            //refresh the grandparent
                            var grandparent = $('#classification_tree_' + primary_classification + ' #n'+new_parent).parent().parent().attr("id");
                            TREE.refresh_by_li('#'+grandparent);
                        }
                        if(!children && $('#classification_tree_' + primary_classification + ' #n'+new_parent).hasClass("closed")) {
                            $('#classification_tree_' + primary_classification + ' #n'+new_parent).removeClass("closed").addClass("leaf");
                            $('#classification_tree_' + primary_classification + ' #n'+new_parent + ' a:first').effect("highlight", { color : "#FF0000" }, 2000);
                            $('#classification_tree_' + primary_classification + ' #n'+parent + ' a:first').effect("highlight", { color : "#00FF00" }, 2000);
                        }
                    }

                    //highlight the new term
                    $('#n'+child + ' a:first').effect("highlight", { color : "#00FF00" });
                    $('#n'+new_child + ' a:first').effect("highlight", { color : "#00FF00" });

                    //refresh the deleted_names & lost_found "trees"
                    if(TREE.jsTree[deleted_names]) {
                        TREE.refresh_tree(deleted_names);
                        $('#classification_tree_' + deleted_names + ' #n'+child + ' a:first').effect("highlight", { color : "#00FF00" });
                    }

                    if(TREE.jsTree[lost_found]) {
                        TREE.refresh_tree(lost_found);
                    }
                    
                }
            }
       });
    }
    
});

function ___getPageSize() {
    var xScroll, yScroll;
    if (window.innerHeight && window.scrollMaxY) {  
        xScroll = window.innerWidth + window.scrollMaxX;
        yScroll = window.innerHeight + window.scrollMaxY;
    } else if (document.body.scrollHeight > document.body.offsetHeight){ // all but Explorer Mac
        xScroll = document.body.scrollWidth;
        yScroll = document.body.scrollHeight;
    } else { // Explorer Mac...would also work in Explorer 6 Strict, Mozilla and Safari
        xScroll = document.body.offsetWidth;
        yScroll = document.body.offsetHeight;
    }
    var windowWidth, windowHeight;
    if (self.innerHeight) { // all except Explorer
        if(document.documentElement.clientWidth){
            windowWidth = document.documentElement.clientWidth; 
        } else {
            windowWidth = self.innerWidth;
        }
        windowHeight = self.innerHeight;
    } else if (document.documentElement && document.documentElement.clientHeight) { // Explorer 6 Strict Mode
        windowWidth = document.documentElement.clientWidth;
        windowHeight = document.documentElement.clientHeight;
    } else if (document.body) { // other Explorers
        windowWidth = document.body.clientWidth;
        windowHeight = document.body.clientHeight;
    }
    // for small pages with total height less then height of the viewport
    if(yScroll < windowHeight){
        pageHeight = windowHeight;
    } else { 
        pageHeight = yScroll;
    }
    // for small pages with total width less then width of the viewport
    if(xScroll < windowWidth){  
        pageWidth = xScroll;        
    } else {
        pageWidth = windowWidth;
    }
    arrayPageSize = new Array(pageWidth,pageHeight,windowWidth,windowHeight);
    return arrayPageSize;
}

function ___getPageScroll() {
    var xScroll, yScroll;
    if (self.pageYOffset) {
        yScroll = self.pageYOffset;
        xScroll = self.pageXOffset;
    } else if (document.documentElement && document.documentElement.scrollTop) {     // Explorer 6 Strict
        yScroll = document.documentElement.scrollTop;
        xScroll = document.documentElement.scrollLeft;
    } else if (document.body) {// all other Explorers
        yScroll = document.body.scrollTop;
        xScroll = document.body.scrollLeft; 
    }
    arrayPageScroll = new Array(xScroll,yScroll);
    return arrayPageScroll;
}

function addCommas(nStr){
    nStr += '';
    x = nStr.split('.');
    x1 = x[0];
    x2 = x.length > 1 ? '.' + x[1] : '';
    var rgx = /(\d+)(\d{3})/;
    while (rgx.test(x1)) {
        x1 = x1.replace(rgx, '$1' + ',' + '$2');
    }
    return x1 + x2;
}

$(window).resize(function() {
    // Get page sizes
    var arrPageSizes = ___getPageSize();
    // Style overlay and show it
    $('#classification-overlay').css({
        width: arrPageSizes[0],
        height: arrPageSizes[1]
    });
    var arrPageScroll = ___getPageScroll();
    $('#classification-message').css({
        top: arrPageScroll[1] + (arrPageSizes[3] / 3.5),
        left: arrPageScroll[0],
        position: 'fixed',
        zIndex: 1001,
        margin: '0px auto',
        width: '100%'
    });
});

/*
* Extension to slide the metadata panel
*/
$.fn.extend({
  slideRight: function() {
    return this.each(function() {
      $(this).animate({width: 'show'});
    });
  },
  slideLeft: function() {
    return this.each(function() {
      $(this).animate({width: 'hide'});
    });
  },
  unwrap: function(expr) {
    return this.each(function() {
      $(this).parents(expr).eq(0).after(this).remove();
    }); 
  },
  delay: function(time,func) {
    return this.each(function() {
      setTimeout(func,time);
    }); 
  }
});

/*
* Helper function to chunk up object retrieved from pinger
*/
function chunk(a, s){
    for(var x, i = 0, c = -1, l = a.length, n = []; i < l; i++)
        (x = i % s) ? n[c][x] = a[i] : n[++c] = [a[i]];
    return n;
}