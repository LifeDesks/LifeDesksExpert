
$.fn.extend({
		classificationTree: function() {
			//override the ActiveX jQuery settings
		    $.ajaxSetup({
			  xhr:function() { return new XMLHttpRequest(); }
		    });

			var vid = Drupal.settings['classification_tags']['vid'];
			var tid = Drupal.settings['classification_tags']['tid'];
			if(Drupal.settings['classification_tags']['ancestry']) {
			  var ancestors = Drupal.settings['classification_tags']['ancestry'].split(",");	
			}

			var conf = {
				    data    : {
				        type    : "xml_flat", // ENUM [json, xml_flat, xml_nested, predefined]
				        method  : "GET",        // HOW TO REQUEST FILES
				        async   : true,        // BOOL - async loading onopen
				        async_data : function (NODE) { return { tid : $(NODE).attr("id") || 0 } }, // PARAMETERS PASSED TO SERVER
				        url     : Drupal.settings.classification_callback_jstree_viewer + vid + "/xml/",        // FALSE or STRING - url to document to be used (async or not)
				        json    : false,        // FALSE or OBJECT if type is JSON and async is false - the tree dump as json
				        xml     : false         // FALSE or STRING
				    },
				    selected    : tid,        // FALSE or STRING or ARRAY
				    opened      : ancestors,           // ARRAY OF INITIALLY OPENED NODES
				    languages   : [],           // ARRAY of string values (which will be used as CSS classes - so they must be valid)
				    path        : Drupal.settings.jstree_path,        // FALSE or STRING (if false - will be autodetected)
				    cookies     : false,        // FALSE or OBJECT (prefix, open, selected, opts - from jqCookie - expires, path, domain, secure)
				    ui      : {
				        dots        : true,     // BOOL - dots or no dots
				        rtl         : false,    // BOOL - is the tree right-to-left
				        animation   : 0,        // INT - duration of open/close animations in miliseconds
				        hover_mode  : true,     // SHOULD get_* functions chage focus or change hovered item
				        scroll_spd  : 4,
				        theme_path  : Drupal.settings.jstree_path + 'themes/',    // Path to themes
				        theme_name  : 'classification',// Name of theme
				        context     : []
				    },
				    rules   : {
				        multiple    : false,    // FALSE | CTRL | ON - multiple selection off/ with or without holding Ctrl
				        metadata    : false,    // FALSE or STRING - attribute name (use metadata plugin)
				        type_attr   : "rel",    // STRING attribute name (where is the type stored if no metadata)
				        multitree   : false,    // BOOL - is drag n drop between trees allowed
				        createat    : "bottom", // STRING (top or bottom) new nodes get inserted at top or bottom
				        use_inline  : false,    // CHECK FOR INLINE RULES - REQUIRES METADATA
				        clickable   : "all",    // which node types can the user select | default - all
				        renameable  : "all",    // which node types can the user select | default - all
				        deletable   : "all",    // which node types can the user delete | default - all
				        creatable   : "all",    // which node types can the user create in | default - all
				        draggable   : "none",   // which node types can the user move | default - none | "all"
				        dragrules   : "all",    // what move operations between nodes are allowed | default - none | "all"
				        drag_copy   : false,    // FALSE | CTRL | ON - drag to copy off/ with or without holding Ctrl
				        droppable   : [],
				        drag_button : "left"
				    },
				    lang : {
				        new_node    : Drupal.t("New folder"),
				        loading     : Drupal.t("Loading ...")
				    },
				    callback    : {             // various callbacks to attach custom logic to
				        // before focus  - should return true | false
				        beforechange: function(NODE,TREE_OBJ) { return true },
				        beforeopen  : function(NODE,TREE_OBJ) { return true },
				        beforeclose : function(NODE,TREE_OBJ) { return true },
				        // before move   - should return true | false
				        beforemove  : function(NODE,REF_NODE,TYPE,TREE_OBJ) { return true }, 
				        // before create - should return true | false
				        beforecreate: function(NODE,REF_NODE,TYPE,TREE_OBJ) { return true }, 
				        // before rename - should return true | false
				        beforerename: function(NODE,LANG,TREE_OBJ) { return true }, 
				        // before delete - should return true | false
				        beforedelete: function(NODE,TREE_OBJ) { return true }, 

				        onJSONdata  : function(DATA,TREE_OBJ) { return DATA; },
				        onselect    : function(NODE,TREE_OBJ) {
					      if(tid != $(NODE).attr("id")) {
					        window.location = Drupal.settings.basePath + 'pages/' + $(NODE).attr("id").replace("n","");
					      }
					    },                  // node selected
				        ondeselect  : function(NODE,TREE_OBJ) { },                  // node deselected
				        onchange    : function(NODE,TREE_OBJ) { },                  // focus changed
				        onrename    : function(NODE,LANG,TREE_OBJ,RB) { },              // node renamed ISNEW - TRUE|FALSE, current language
				        onmove      : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) { }, // move completed (TYPE is BELOW|ABOVE|INSIDE)
				        oncopy      : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) { }, // copy completed (TYPE is BELOW|ABOVE|INSIDE)
				        oncreate    : function(NODE,REF_NODE,TYPE,TREE_OBJ,RB) { }, // node created, parent node (TYPE is createat)
				        ondelete    : function(NODE, TREE_OBJ,RB) { },                  // node deleted
				        onopen      : function(NODE, TREE_OBJ) { },                 // node opened
				        onopen_all  : function(TREE_OBJ) { },                       // all nodes opened
				        onclose     : function(NODE, TREE_OBJ) { },                 // node closed
				        error       : function(TEXT, TREE_OBJ) { },                 // error occured
				        // double click on node - defaults to open/close & select
				        ondblclk    : function(NODE, TREE_OBJ) { TREE_OBJ.toggle_branch.call(TREE_OBJ, NODE); TREE_OBJ.select_branch.call(TREE_OBJ, NODE); },
				        // right click - to prevent use: EV.preventDefault(); EV.stopPropagation(); return false
				        onrgtclk    : function(NODE, TREE_OBJ, EV) { },
				        onload      : function(TREE_OBJ) { },
				        onfocus     : function(TREE_OBJ) { },
				        ondrop      : function(NODE,REF_NODE,TYPE,TREE_OBJ) {}
				    }
			};
		  	var tree = $.tree_create();
		    $("#classification_tree_viewer").html('<span class="classification_loading">'+Drupal.t("Loading...")+ '</span>').fadeIn('slow', function(){
			  tree.init($("#classification_tree_viewer"), $.extend({},conf));
		    });

			//search functions
		    $('#edit-classification-submit-' + vid).click(function() {
		      var $search = $('.classification-search-form-text').val();
		      $.tree_reference('classification_tree_viewer').search($search);
			  $('#classification_tree_viewer').scrollTo( $('.search:last'),1000,{offset:-100} ).scrollTo( $('.search:last'),500,{axis:'x'} );
		    });
		}
});