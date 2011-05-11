$(function() {
  //relocate the tree
  var tree_html = $('.classification_tree_panel');
  $('.node-form').parent().css("position","relative").append(tree_html);

  $('.expand').show();
  $('.expand').click(function() {
    $('.classification_tree_div').animate({
      "width": "400px"
    },1500);
    $('#classification_tree_viewer').animate({
      "height" : "500px"
    },1500);
    $(this).hide();
    $('.contract').show();
  });
  $('.contract').click(function() {
    $('.classification_tree_div').animate({
      "width": "230px"
    },500);
    $('#classification_tree_viewer').animate({
      "height" : "260px"
    },1500);
    $(this).hide();
    $('.expand').show();
  });

  var vid = Drupal.settings['classification_tags']['vid'];
  var nid = Drupal.settings['nid'];
  var _url = (!nid) ? false : Drupal.settings.basePath + "classification/js_tree_checked/" + vid + "/" + nid;
  var checked = Drupal.settings['classification_tags']['checked'];
  if(Drupal.settings['classification_tags']['ancestry']) {
    var ancestors = Drupal.settings['classification_tags']['ancestry'].split(",");  
  }
        
  var conf =  {
    data    : {
      type    : "xml_flat", // ENUM [json, xml_flat, xml_nested, predefined]
      method  : "GET",        // HOW TO REQUEST FILES
      async   : true,        // BOOL - async loading onopen
      async_data : function (NODE) { return { tid : $(NODE).attr("id") || 0 } }, // PARAMETERS PASSED TO SERVER
        url     : Drupal.settings.classification_callback_jstree_viewer + vid + "/xml/",        // FALSE or STRING - url to document to be used (async or not)
        json    : false,        // FALSE or OBJECT if type is JSON and async is false - the tree dump as json
        xml     : false         // FALSE or STRING
      },
      selected    : false,        // FALSE or STRING or ARRAY
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
        theme_name  : 'classification_checkbox',// Name of theme
        context     : [ ]
      },
      lang : {
        loading     : Drupal.t("Loading ...")
      },
      callback  : {
        onopen : function(NODE, TREE_OBJ) {
        },
        onchange : function (NODE, TREE_OBJ) {
            var $this = $(NODE).children("a:eq(0)");
            if($this.size() == 0) {
              TREE_OBJ.container.find("a").addClass("unchecked");
            }
            $this.removeClass("clicked");
            if($this.hasClass("checked")) {
                $this.removeClass("checked").addClass("unchecked");
            }
            else {
                $this.removeClass("unchecked").addClass("checked");
            }
            __rebuildTags();
       }
     }      
  };

  var TREE = $.tree_create();
  TREE.init($("#classification_tree_viewer_" + vid), $.extend({},conf));
                    
  //search functions
  $('#edit-classification-submit-' + vid).click(function() {
    var $search = $('#edit-classification-search-' + vid).val();
    $.tree_reference('classification_tree_viewer_' + vid).search($search);
    $('#classification_tree_viewer_' + vid).scrollTo( $('.search:last'),1000,{offset:-100} ).scrollTo( $('.search:last'),500,{axis:'x'} );
  });
        
});

function __openBranches(vid,NODE) {
  $.tree_reference('classification_tree_viewer_' + vid).open_branch(NODE);  
}

function __rebuildTags() {
  var vid = Drupal.settings['classification_tags']['vid'];
  var $tag_holder = $('#edit-taxonomy-classification-multitag-' + vid);
  $tag_holder.val('');
  $("a.checked").each(function(i) {
    var $term = $(this).parent("li").attr("id").replace("n","");
    if($tag_holder.val() == '' || $tag_holder.val() == ',') {
      $tag_holder.val($term);   
    }
    else {
      $tag_holder.val($tag_holder.val() + ',' + $term);
    }
  });   
}