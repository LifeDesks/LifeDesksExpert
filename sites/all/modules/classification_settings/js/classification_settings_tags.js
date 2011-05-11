function classification_settings() {
    this.tags_activate = function(context,vid,option) {
	  var _this = this;
	  if ($(context).length == 1) {
	    var tagarea = _this.tags_widget(context,vid,option);
	    $(context).before(tagarea);
	    Drupal.behaviors.autocomplete(document);
	  }
	  $('.classification-add-tag:not(.classification-tag-processed)').click(function() {
	    var $val = $(this).prev().val();
	    _this.tags_add(context,$val,vid,option);
	    $(this).prev().val('');
	  }).addClass('classification-tag-processed');

	  if ($.browser.mozilla) {
	    $('.classification-tag-entry:not(.classification-tag-processed)').keypress(_this.tags_check_enter).addClass('classification-tag-processed');
	  }
	  else {
	    $('.classification-tag-entry:not(.classification-tag-processed)').keydown(_this.tags_check_enter).addClass('classification-tag-processed');
	  }
	  $(context).hide();	
    };
    this.tags_check_enter = function(event) {
	  if (event.keyCode == 13) {
	    $('#autocomplete').each(function () {
	      this.owner.hidePopup();
	    })
	    $(this).next().click();
	    event.preventDefault();
	    return false;
	  }	
    };
    this.tags_callback = function(context,tids,vid,option) {
	  var _this = this;
	  $.post(Drupal.settings.classification_callback_base_url + '/buildnames/', { 'tids':tids }, function(content) {
		switch(option) {
		  case 'single':
		    for(var i=0;i<content.data.length;i++) {
		        $('#classification-tag-edit-' + vid).hide().next().hide();
		        $(context).prev().children('.classification-tag-holder').children().remove();
		        $(context).prev().children('.classification-tag-holder').append('<div class="classification-tag-single"><span class="classification-tag-tid">' + content.data[i].tid + '</span><span class="classification-tag-text">' +
		          content.data[i].name + '</span><span class="classification-edit-tag">' + Drupal.t('edit') + '</span></div>');
		        $('.classification-edit-tag:not(.classification-tag-processed)').click(function() {
		          $(this).parent().remove();
		          _this.tags_update(context,vid,option);
		          $('#classification-tag-edit-' + vid).show().next().show();
		        }).addClass('classification-tag-processed');
			}
			$(context).val('true');
		  break;

		  case 'multiple':
		    for(var i=0;i<content.data.length;i++) {
			  $(context).prev().children('.classification-tag-holder').children().remove();
	          $(context).prev().children('.classification-tag-holder').append('<div class="classification-tag"><span class="classification-tag-tid">' + content.data[i].tid + '</span><span class="classification-tag-text">' +
	            content.data[i].name + '</span><span class="classification-remove-tag" title="' + Drupal.t('remove') + '">&nbsp;</span></div>');
	          $('.classification-remove-tag:not(.classification-tag-processed)').click(function() {
	            $(this).parent().remove();
	            _this.tags_update(context,vid,option);
	          }).addClass('classification-tag-processed');
	        }
	        $(context).val('true');
		  break;

		  case 'tree':
		  break;
		}	
	  }, "json");	
    };
    this.tags_add = function(context,v,vid,option) {
	  var _this = this;
	  if(!v) {
	    return;	
	  }
	  $.post(Drupal.settings.classification_callback_base_url + '/checkterm/', { 'vid':vid, 'name':v }, function(content) {
	    switch(content.data.length) {
		  case 0:
		    var $message = $('#classification-settings-message');
		    $('#classification-settings-overlay').fadeIn();
		    $message.fadeIn('slow').html('<div id="classification-settings-warning" class="messages warning">' + content.message + '<br /><input type="submit" id="classification_settings_ok" class="form-submit" value="' + Drupal.t('OK') + '"></div>');
		    $('#classification_settings_ok').click(function() { _this.hide_message(); return false;});
		   break;
		  case 1:
		    if (jQuery.trim(v) != '') {
			  switch(option) {
			    case 'single':
			      $('#classification-tag-edit-' + vid).hide().next().hide();
		          $(context).prev().children('.classification-tag-holder').append('<div class="classification-tag-single"><span class="classification-tag-tid">' + content.data[0].tid + '</span><span class="classification-tag-text">' +
		            jQuery.trim(v) + '</span><span class="classification-edit-tag">' + Drupal.t('edit') + '</span></div>');
			      $('.classification-edit-tag:not(.classification-tag-processed)').click(function() {
			        $(this).parent().remove();
			        _this.tags_update(context,vid,option);
			        $('#classification-tag-edit-' + vid).show().next().show();
			      }).addClass('classification-tag-processed');
			      break;
			    case 'multiple':
		          $(context).prev().children('.classification-tag-holder').append('<div class="classification-tag"><span class="classification-tag-tid">' + content.data[0].tid + '</span><span class="classification-tag-text">' +
		            jQuery.trim(v) + '</span><span class="classification-remove-tag" title="' + Drupal.t('remove') + '">&nbsp;</span></div>');
			      $('.classification-remove-tag:not(.classification-tag-processed)').click(function() {
			        $(this).parent().remove();
			        _this.tags_update(context,vid,option);
			      }).addClass('classification-tag-processed');
			      break;
			    case 'tree':
			      break;	
			  }
		    }
		   break;
		  default:
		   var $message = $('#classification-settings-message');
		    $('#classification-settings-overlay').fadeIn();
		    var output = '<form><ul id="classification-settings-selection">';
		    for(var i=0;i<content.data.length;i++) {
			  output += '<li><input type="radio" name="classification_selections" value="' + content.data[i].tid + '" rel="' + content.data[i].ancestry + '">'
			  output += content.data[i].name + ' (' + content.data[i].ancestry + ')';
			  output += '</input></li>';
		    }
		    output += '</ul></form>';
		    $message.fadeIn('slow').html('<div id="classification-settings-warning" class="messages warning">' + content.message + '<div>' + output + '<input type="submit" id="classification_settings_ok" class="form-submit" value="' + Drupal.t('Submit') + '"><a id="classification_settings_cancel">' + Drupal.t('Cancel') + '</a></div></div>');
	        $('#classification_settings_cancel').click(function() { _this.hide_message(); return false;});
	        $('#classification_settings_ok').click(function() {
		      var $val = $("input[name='classification_selections']:checked").val();
		      var $rel = $("input[name='classification_selections']:checked").attr("rel");
		      if($val>0) {
			      switch(option) {
				    case 'single':
			          $('#classification-tag-edit-' + vid).hide().next().hide();
		              $(context).prev().children('.classification-tag-holder').append('<div class="classification-tag-single"><span class="classification-tag-tid">' + $val + '</span><span class="classification-tag-text" title="' + $rel + '">' +
		                jQuery.trim(v) + '</span><span class="classification-edit-tag">edit</span></div>');
			          $('.classification-edit-tag:not(.classification-tag-processed)').click(function() {
			            $(this).parent().remove();
			            _this.tags_update(context,vid,option);
			            $('#classification-tag-edit-' + vid).show().next().show();
			          }).addClass('classification-tag-processed');
			          _this.tags_update(context,vid,option);
				      break;
				    case 'multiple':
				      if (jQuery.trim(v) != '') {
				        $(context).prev().children('.classification-tag-holder').append('<div class="classification-tag"><span class="classification-tag-tid">' + $val + '</span><span class="classification-tag-text" title="' + $rel + '">' +
				          jQuery.trim(v) + '</span><span class="classification-remove-tag">&nbsp;</span></div>');
				        $('.classification-remove-tag:not(.classification-tag-processed)').click(function() {
						  $(this).parent().remove();
						  _this.tags_update(context,vid,option);
						}).addClass('classification-tag-processed');
				      }
				      _this.tags_update(context,vid,option);
				      break;
				    case 'tree':
				    break;			
			      }
			      _this.hide_message();
		      }
		   });
	    }
	    _this.tags_update(context,vid,option);
	  }, "json");	
    };
    this.tags_update = function(context,vid,option) {
	  $(context).val('true');
	  switch(option) {
		case 'single':
		  var $tag_holder = '.classification-singletag-tid-' + vid;
	      $(context).prev().children('.classification-tag-holder').children().children('.classification-tag-tid').each(function(i) {
	        if(i==0) {
	          $($tag_holder).val($(this).text());
	        }
	        else {
	          $($tag_holder).val(
		        $($tag_holder).val() + ',' + $(this).text()
		      );
	        }
	      });
	      if($(context).prev().children('.classification-tag-holder').children().html()==null) {
		    $($tag_holder).val('');
		    $(context).val('');
	      }
		  break;
	    case 'multiple':
	      var $tag_holder = '.classification-multitag-tids-' + vid;
	      $(context).prev().children('.classification-tag-holder').children().children('.classification-tag-tid').each(function(i) {
	        if(i==0) {
	          $($tag_holder).val($(this).text());
	        }
	        else {
	          $($tag_holder).val(
		        $($tag_holder).val() + ',' + $(this).text()
		      );
	        }
	      });
	      if($(context).prev().children('.classification-tag-holder').children().html()==null) {
		    $($tag_holder).val('');
		    $(context).val('');
	      }
	      break;
	    case 'tree':
	      break;
	  }	
    };
    this.tags_widget = function(context,vid,option) {
	  switch(option) {
	    case 'single':
	      return '<div id="' + context.replace('.','') + '">' +
	        '<div class="classification-tag-holder"></div>' +
	        '<input type="text" class="classification-tag-entry form-autocomplete" size="40" id="classification-tag-edit-' + vid + '" />' +
	        '<input type="button" value="' + Drupal.t('add') + '" class="classification-add-tag">' +
	        '<input class="autocomplete" type="hidden" id="classification-tag-edit-' + vid + '-autocomplete" ' +
	        'value="http://' + document.domain + Drupal.settings.classification_callback_base_url + '/autocomplete/' + vid + '" disabled="disabled" />' +
	        '</div>';
	      break;
	    case 'multiple':
	      return '<div id="' + context.replace('.','') + '">' +
	        '<input type="text" class="classification-tag-entry form-autocomplete" size="40" id="classification-tag-edit-' + vid + '" />' +
	        '<input type="button" value="' + Drupal.t('add') + '" class="classification-add-tag">' +
	        '<input class="autocomplete" type="hidden" id="classification-tag-edit-' + vid + '-autocomplete" ' +
	        'value="http://' + document.domain + Drupal.settings.classification_callback_base_url + '/autocomplete/' + vid + '" disabled="disabled" />' +
	        '<div class="classification-tag-holder"></div>' +
	        '</div>';
	      break;
	    case 'tree':
	      break;
	  }	
    };
    this.hide_message = function() {
	  $('#classification-settings-message').fadeOut();
	  $('#classification-settings-overlay').fadeOut();	
    }; 
}

Drupal.behaviors.tagger = function (context) {
  CLASS = new classification_settings();
  TREE = {};

  jQuery.each(Drupal.settings['classification_tags'], function(i, v) {
	switch(v.opt) {
	  case 'single':
	    var _v = '.classification-singletag-' + v.vid;
        if (!$(_v).parent('.form-item').hasClass('classification-tags-processed')) {
          CLASS.tags_activate(_v,v.vid,'single');
          $(_v).parent('.form-item').addClass('classification-tags-processed');
        }
		if($('.classification-singletag-tid-' + v.vid).val()) {
		  CLASS.tags_callback(_v,$('.classification-singletag-tid-' + v.vid).val(),v.vid,v.opt);
	    }
	    break;
	  case 'multiple':
	    var _v = '.classification-multitag-' + v.vid;
        if (!$(_v).parent('.form-item').hasClass('classification-tags-processed')) {
          CLASS.tags_activate(_v,v.vid,'multiple');
          $(_v).parent('.form-item').addClass('classification-tags-processed');
        }
		if($('.classification-multitag-tids-' + v.vid).val()) {
		  CLASS.tags_callback(_v,$('.classification-multitag-tids-' + v.vid).val(),v.vid,v.opt);
	    }
        break;
      case 'tree':
        if( $('.classification-multitag-tids-' + v.vid).length > 0 ) {
	        var $init = $('#edit-taxonomy-classification-multitag-loader-' + v.vid);
	        var ancestors = '';
			if($('.classification-multitag-tids-' + v.vid).val()) {
			  ancestors = $('#edit-taxonomy-classification-multitag-ancestry-' + v.vid).val().split(",");
			}

			var conf =  {
			  data    : {
				type    : "xml_flat", // ENUM [json, xml_flat, xml_nested, predefined]
				method  : "GET",        // HOW TO REQUEST FILES
				async   : true,        // BOOL - async loading onopen
				async_data : function (NODE) { return { tid : $(NODE).attr("id") || 0 } }, // PARAMETERS PASSED TO SERVER
			      url     : Drupal.settings.classification_callback_jstree_elements + v.vid + "/xml/",        // FALSE or STRING - url to document to be used (async or not)
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
				callback	: {
				  onopen : function(NODE, TREE_OBJ) {
				  },
				  onchange : function (NODE, TREE_OBJ) {
					$init.val('false');
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
					__rebuildTags(v.vid);
			      },
			      onload : function(TREE_OBJ) {
				    $checks = $('#edit-taxonomy-classification-multitag-' + v.vid);
				    var checked = $checks.val().split(",");
				    for(i=0;i<checked.length;i++) {
					  var NODE = '#n' + checked[i];
					  $(NODE).children("a").addClass("checked");
					  var $this = $(NODE).is("li") ? $(NODE) : $(NODE).parent();
					  if($this.children("a.unchecked").size() == 0) {
				        TREE_OBJ.container.find("a").addClass("unchecked");
					  }
					}
			      }
			    }		
			  };

			  TREE[v.vid] = $.tree_create();
			  TREE[v.vid].init($("#classification_tree_viewer_" + v.vid), $.extend({},conf));
			
			  $('#edit-classification-submit-' + v.vid).click(function() {
		        var $search = $('.classification-search-form-text').val();
			    $.tree_reference('classification_tree_viewer_' + v.vid).search($search);
			    $('#classification_tree_viewer_' + v.vid).scrollTo( $('.search:last'),1000,{offset:-100} ).scrollTo( $('.search:last'),500,{axis:'x'} );
		      });
		
          }

          break; 
	  }
  });
}

function __openBranches(vid,NODE) {
  $.tree_reference('classification_tree_viewer_' + vid).open_branch(NODE);	
}

function __rebuildTags(vid) {

  var $tag_holder = $('#edit-taxonomy-classification-multitag-' + vid);
  var $ancestry_holder = $('#edit-taxonomy-classification-multitag-ancestry-' + vid);
  var $init = $('#edit-taxonomy-classification-multitag-loader-' + vid);
  if($init.val() == 'false') {
    $tag_holder.val('');
    $ancestry_holder.val('');
  }
  var ancestry_array = [];
  $("a.checked").each(function(i) {
    var term = $(this).parent("li").attr("id").replace("n","");
	if($tag_holder.val() == '' || $tag_holder.val() == ',') {
	  $tag_holder.val(term);	
	}
	else {
	  $tag_holder.val($tag_holder.val() + ',' + term);
	}
	$(this).parents("li").not('.leaf').each(function(i) {
		var ancestor = $(this).attr("id");
		ancestry_array.push(ancestor);
	});
	ancestry_array = __uniqueArr(ancestry_array); 
	$ancestry_holder.val(ancestry_array);
  });	
}

function __uniqueArr(a) {
 temp = new Array();
 for(i=0;i<a.length;i++){
  if(!__contains(temp, a[i])){
   temp.length+=1;
   temp[temp.length-1]=a[i];
  }
 }
 return temp;
}
 
function __contains(a, e) {
 for(j=0;j<a.length;j++)if(a[j]==e)return true;
 return false;
}


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
	if (self.innerHeight) {	// all except Explorer
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
};

function ___getPageScroll() {
	var xScroll, yScroll;
	if (self.pageYOffset) {
		yScroll = self.pageYOffset;
		xScroll = self.pageXOffset;
	} else if (document.documentElement && document.documentElement.scrollTop) {	 // Explorer 6 Strict
		yScroll = document.documentElement.scrollTop;
		xScroll = document.documentElement.scrollLeft;
	} else if (document.body) {// all other Explorers
		yScroll = document.body.scrollTop;
		xScroll = document.body.scrollLeft;	
	}
	arrayPageScroll = new Array(xScroll,yScroll);
	return arrayPageScroll;
};

$(function() {
    //override the ActiveX jQuery settings
    $.ajaxSetup({
	  xhr:function() { return new XMLHttpRequest(); }
    });

	//set the interface
	$('body').append('<div id="classification-settings-overlay"></div><div id="classification-settings-message"></div>');
	var arrPageSizes = ___getPageSize();
	$('#classification-settings-overlay').css({
		backgroundColor: 'black',
		opacity: 0.66,
		width: arrPageSizes[0],
		height: arrPageSizes[1]
	});
	var arrPageScroll = ___getPageScroll();
	$('#classification-settings-message').css({
		top: arrPageScroll[1] + (arrPageSizes[3] / 3),
		left: arrPageScroll[0],
		position: 'absolute',
		zIndex: 100,
		margin: '0px auto',
		width: '100%'
	});
});

$(window).resize(function() {
	// Get page sizes
	var arrPageSizes = ___getPageSize();
	// Style overlay and show it
	$('#classification-settings-overlay').css({
		width: arrPageSizes[0],
		height: arrPageSizes[1]
	});
	var arrPageScroll = ___getPageScroll();
	$('#classification-settings-message').css({
		top: arrPageScroll[1] + (arrPageSizes[3] / 3),
		left: arrPageScroll[0],
		position: 'absolute',
		zIndex: 100,
		margin: '0px auto',
		width: '100%'
	});
});