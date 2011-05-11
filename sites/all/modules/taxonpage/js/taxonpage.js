$(function(){

  //flag tooltips
  $('.flag-info').bt({
	positions: 'top',
	trigger: 'click',
	width: 220,
	centerPointX: .9,
	spikeLength: 65,
	spikeGirth: 40,
	padding: 15,
	cornerRadius: 25,
	fill: '#FFF',
	strokeStyle: '#ABABAB',
	strokeWidth: 1
  });
  
  //override the ActiveX jQuery settings
  $.ajaxSetup({
	xhr:function() { return new XMLHttpRequest(); }
  });

  //biblio parser:  commented out for further testing
/*
  $('.biblio-entry').refParser({
	pid : Drupal.settings.lifedesk_biblio_pid, 
	parserUrl : Drupal.settings.lifedesk_biblio_parserurl,
	iconPath : Drupal.settings.lifedesk_biblio_iconpath,
	timeout : 30000
  });
*/

  var toggled=false;
  $('.classification_tree').click(function() {
    if(toggled==false) {
	  if($('#classification_tree_viewer ul').length == 0) {
	    $('#classification_tree_viewer').classificationTree();	
	  }
	  $(this).children('.classification_show').hide();
	  $(this).children('.classification_hidden').show();
	  $('#classification_tree_wrapper').fadeIn("slow");
	  toggled = true;
    }
    else {
	  $(this).children('.classification_show').show();
	  $(this).children('.classification_hidden').hide();
	  $('#classification_tree_wrapper').hide();
	  toggled = false;
    }
  });

  $('.expand').show();
  $('.expand').click(function() {
   $('#classification_tree_div').animate({
    "width": "400px"
   },1500);
   $('#classification_tree_viewer').animate({
    "height" : "500px"
   },1500);
   $(this).hide();
   $('.contract').show();
  });
  $('.contract').click(function() {
   $('#classification_tree_div').animate({
    "width": "230px"
   },500);
   $('#classification_tree_viewer').animate({
    "height" : "260px"
   },1500);
   $(this).hide();
   $('.expand').show();
  });

  $('ul.gallery').show();
  $('ul.gallery').galleria({
			history   : false,
			clickNext : false,
			insert    : '#main_image',
			onImage   : function(image,caption,thumb) {
				var index = $('ul.gallery li').index(thumb.parent());
				var caption_data = Drupal.settings.taxonpageImageData[index];
				if(! ($.browser.mozilla && navigator.appVersion.indexOf("Win")!=-1) ) {
					image.attr({alt:caption_data.title,title:caption_data.title});
					image.css('display','none').fadeIn(1000);
				}
				var _status = caption_data.status == 0 ? '<li class="taxonpage_draft">' + Drupal.t('Unpublished') + '</li>' : '';
				var _perm = '<li class="perma_link"><a href="#">' + Drupal.t('Permalink') + '</a></li>';
				var _edit = caption_data.permission ? '<li class="edit_image"><a href="' + Drupal.settings.basePath + 'node/' + caption_data.node + '/edit">' + Drupal.t('Edit') + '</a></li>': '';
                var _comment = caption_data.comment ? '<li class="comment_balloon"><a href="' + Drupal.settings.basePath + 'node/' + caption_data.node + '">' + Drupal.t('Comment') + '</a> ('+caption_data.comment_count+')</li>': '';
                var _caption = '<div class="image_caption"><div class="caption_tools"><ul class="taxonpage_inline_links">' + _status + '<li>' + caption_data.cclicense + '</li>' + _perm + _edit + _comment + '</ul>';
                _caption += '<div class="perma-link-popup"><input type="text" value="http://' + document.domain + '/node/' + caption_data.node + '" rel="http://' + document.domain + '/node/' + caption_data.node + '" size="30"></input><span class="perma-link-close"><a href="#">' + Drupal.t('close') + '</a></span></div>';
                _caption += '</div><h3 class="taxonpage_image_title">' + caption_data.title + '</h3>';
				_caption += '<div class="taxonpage_image_credit">';
				_caption += (caption_data.credit) ? '<b>' + Drupal.t('Photographer') + ':</b> ' + caption_data.credit : '';
                _caption += (caption_data.rights && (caption_data.license != 'publicdomain')) ? ' <b>' + Drupal.t('Rights holder') + ':</b> ' + caption_data.rights : '';
                _caption += (caption_data.author) ? ' <b>' + Drupal.t('Submitted by') + ':</b> ' + caption_data.author : '';
                _caption += '</div>';
                _caption += (caption_data.taxon_tid != Drupal.settings.tid) ? '<div class="taxonpage_relations_content">' + Drupal.t('Submitted as') + ' <span>' + caption_data.taxon_name + '</span></div>' : '';
				caption.html(_caption);
				
				//permalink handling
				var $permaLink = $('.perma-link-popup input');
				$permaLink.blur(function() {
				  $(this).val($permaLink.attr("rel"));	
				});
				
			    $('.perma_link a').click(function() {
				  $('.perma-link-popup').show();
				  return false;
			    });

			    $('.perma-link-close a').click(function() {
				  $('.perma-link-popup').hide();
				  return false;
			    });
			
				caption.css('display','none').show();
				var _li = thumb.parents('li');
				_li.siblings().children('img.selected').fadeTo(500,0.3);
				thumb.fadeTo('fast',1).addClass('selected');
				image.attr('alt',caption_data.title);
				
				if(caption_data.status == 0) {
					$('div.galleria_wrapper').addClass('draft');
					$('.caption_tools').fadeIn(500);
				}
				else {
					$('div.galleria_wrapper').removeClass('draft');
				}
			},
			onThumb : function(thumb) {
				var index = $('ul.gallery li').index(thumb.parent());
				var _li = thumb.parents('li');
				var _fadeTo = _li.is('.active') ? '1' : '0.3';
				var caption_data = Drupal.settings.taxonpageImageData[index];
				thumb.attr({alt:caption_data.title,title:caption_data.title}).css({display:'none',opacity:_fadeTo}).fadeIn(1500);
				thumb.hover(
					function() { thumb.fadeTo('fast',1); },
					function() { _li.not('.active').children('img').fadeTo('fast',0.3); }
				)
			}
		});
		
  $('#taxonpage_gallery').jcarousel({
	scroll : 4
  });

  //handling of attachments for biblio items
  $('.attachments').bt({
	  contentSelector: "$(this).next().html()",
	  trigger: 'click',
	  shrinkToFit: true,
	  fill: '#FFF',
	  width: '255px',
	  strokeStyle: '#ABABAB',
	  strokeWidth: 1
  });

/*
  $('.biblio-entry').append('<img src="' + bioGUIDicon[0] + '" class="ref_icon" alt="Search!" title="Search!" />');
  $('.ref_icon').css({'cursor':'pointer','border':'0px','height':'16px','width':'16px','vertical-align':'center'}).click(function() {
    var _this = this;
	
	//remove the attachments html then add it back again such that ref text is "clean"
    var attachments = $(this).parent().find("div.attachments_popup");
    var attachments_text = $(attachments).html();
    $(attachments).html('');
    var ref = $(this).parent().text();
    $(attachments).html(attachments_text);

    $(this).attr({src : bioGUIDicon[1], alt : Drupal.t('Looking for reference...'), title : Drupal.t('Looking for reference...')});
    $.getJSON(bioGUIDurl + '?output=json&q=' + escape(ref) + '&callback=?', {}, function(data) {
	  switch(data.status) {
	    case 'ok':
	      var doi = data.record.doi;
		  var hdl = data.record.hdl;
		  var url2 = data.record.url;
		  var atitle = data.record.atitle;
		  var glink = "http://scholar.google.com/scholar?q="+escape(atitle)+"&as_subj=bio";

		  if (doi == null && hdl == null && url2 == null && atitle != null) {
			$(_this).attr({src : bioGUIDicon[9], alt : Drupal.t('Search Google Scholar'), title : Drupal.t('Search Google Scholar')}).unbind('click').click(function() {
				window.location = glink;
			});
		  }
		  else {		
			if (doi != null) {
				var link = "http://dx.doi.org/"+doi;
                $(_this).attr({src : bioGUIDicon[4], alt : Drupal.t('To publisher...'), title : Drupal.t('To publisher...')}).unbind('click').click(function() {
	              window.location = link;
                });
			}
			if (hdl != null) {
				var link = "http://hdl.handle.net/"+hdl;
                $(_this).attr({src : bioGUIDicon[5], alt : Drupal.t('To publisher...'), title : Drupal.t('To publisher...')}).unbind('click').click(function() {
	              window.location = link;
                });
			}
			if (url2 != null) {
				var ext = Right(url2,4);
				var img = bioGUIDicon[3];
				var alt = "HTML";

				if (ext == ".pdf") {
					img = bioGUIDicon[2];
					alt = "PDF";
				}						
                $(_this).attr({src : img, alt : alt, title : alt}).click(function() {
	              window.location = url2;
                });
			}
		}
	    break;
	    case 'failed':
	      $(_this).attr({src : bioGUIDicon[7], alt : Drupal.t('Unable to parse reference'), title : Drupal.t('Unable to parse reference')}).css('cursor','default').unbind('click');
	    break;	
	  }
    });
  });
*/

  $('#taxonpage_author_select').change(function() {
	var tid = Drupal.settings.tid.replace("#n","");
    var val = $('#taxonpage_author_select option:selected').val();
    if(val && val>0) {
	  window.location.href = Drupal.settings.basePath + "pages/" + tid + "/user/" + val;
    }
    else if (val && val==0) {
	  window.location.href = Drupal.settings.basePath + "pages/" + tid;
    }
    else {}
  });

  $('.galleria_wrapper').hover(
	  function() { $('.caption_tools').fadeIn(500); }
  );

  // when language chosen, do call-back to get empty chapters per language
  $('#taxonpage_language_select').change(function() {
	var lang = $('#taxonpage_language_select option:selected').val();
	if(lang) {
		$('#taxonpage_chapter_selections').html(Drupal.t("Loading..."));
		$.get(Drupal.settings.basePath + "taxonpage/language/chapters/" + Drupal.settings.tid + "/" + lang, {}, function(data) {
		  $('#taxonpage_chapter_selections').html(data);
		  $('#taxonpage_chapter_select').change(function() {
			 var val = $(this).val().split(":");
		     window.location.href = Drupal.settings.basePath + 'node/' + val[0] + '/edit?chapter=' + val[1];	
		  });	
		}, "html");	
	}
	else {
	  $('#taxonpage_chapter_selections').html('');
	}
  });

});