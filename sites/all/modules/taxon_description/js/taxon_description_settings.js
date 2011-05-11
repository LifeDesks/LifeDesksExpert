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
};

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
};

jQuery.fn.closestPrev = function(selector) {
    var current = $(this);
    var abort = false;
    while (!abort) {
        if (current.prev().length == 0) {
            abort = true;
            current = $([]);
        }
        else {
            current = current.prev();
            if (current.is(selector)) {
                abort = true;
            }
        }
    }
    return current;
};

$(function() {

  $.ajaxSetup({
    xhr:function() { return new XMLHttpRequest(); }
  });

  //requires jQuery UI here
  $("#contentLeft ul").sortable({
    opacity: 0.6,
    cursor: 'move',
    placeholder: 'ui-state-highlight',
    items: 'li:not(.ui-state-stable)',
    cancel: ".ui-state-disabled",
    update: function() {
      var order = $('.stable').attr("rel")+'&'+$(this).sortable("serialize");
      $.post(Drupal.settings.chapter_config_callback + "reorder_chapter/", {"order" : order, "action" : "updateRecordsListings"}, function(data) { TD.displayMessage(data);}, "json");
    }
  });

  TD = {};

  TD.addChapter = function(pid) {
    var _this = this;
    var parent_name = $('#tid_'+pid).find("strong").text();
    var chapter_form = '<div id="taxon-description-chapter-newform">';
    chapter_form += '<div id="taxon-description-close" class="taxon-description-close"><a href="#" onclick="return false;">' + Drupal.t('close') + '</a></div>';
    chapter_form += '<form id="chapter-form">';
    chapter_form += '<h3>' + parent_name + '</h3>';
    var title = (pid == 0) ? Drupal.t('Add new heading') : Drupal.t('Add new chapter');
    chapter_form += '<fieldset><legend>' + title + '</legend><div id="taxon-description-addchapter-wrapper">';

    var languages = Drupal.settings.lang;

    $.each(languages, function(key, value) {

    chapter_form += '<fieldset class= "collapsible collapsed"><legend>'+value+'</legend><div>';

    chapter_form += '<div id="chapter-name-wrapper" class="form-item">';
    chapter_form += '<label for="chapter-name-'+key+'">' + Drupal.t('Chapter title') + ': <span class="form-required" title="' + Drupal.t('This field is required.') + '">*</span></label>';
    chapter_form += '<input id="chapter-name-'+key+'" class="form-text required" size="60" maxlength="100" type="text" name="taxon-description['+key+'][name]" value=""></input>';
    chapter_form += '</div>';

    chapter_form += '<div id="chapter-description-wrapper" class="form-item">';
    chapter_form += '<label for="chapter-description-'+key+'">' + Drupal.t('Description') + '</label>';
    chapter_form += '<textarea id="chapter-description-'+key+'" class="form-textarea" rows="5" cols="60" name="taxon-description['+key+'][description]"></textarea>';
    chapter_form += '</div>';

    chapter_form += '</div></fieldset>';
    var parent_id = 0;
    if(key==='en'){
      parent_id = pid;
    }
    chapter_form += '<input type="hidden" id="taxon-description-parent" name="taxon-description['+key+'][parent]" value=' + parent_id + '></input>';
    chapter_form += '<input type="hidden" id="taxon-description-language" name="taxon-description['+key+'][language]" value=' + key + '></input>';

    });

    chapter_form += '</div></fieldset>';

    chapter_form += '<input type="submit" id="taxon-description-submit" class="form-submit" value="' + Drupal.t('Save') + '" onclick="javascript:return false;">';
    chapter_form += '<a id="taxon-description-cancel">' + Drupal.t('Cancel') + '</a>';
    chapter_form += '</form>';
    chapter_form += '</div>';

    this.showMessage(chapter_form);
    Drupal.attachBehaviors($('#taxon-description-addchapter-wrapper'));

    $('#taxon-description-cancel').click(function() {
      _this.hideMessage();
    });

    $('#taxon-description-close').click(function() {
      _this.hideMessage();
    });

    $('#taxon-description-submit').click(function() {
          var valid = true;
          $('#taxon-description-addchapter-wrapper .required').each(function() {
            if($(this).val() == "" || $(this).val() == Drupal.t("Undefined")) {
              valid = false;
              Drupal.toggleFieldset($(this).parent().parent().parent().parent().addClass('collapsed')[0]);
              $(this).addClass("error").keyup(function() { $(this).removeClass("error"); });
            }
           });
           if(valid) {
                  var new_name = $('#chapter-name-en').val();
          var new_description = ($('#chapter-description-en').val()) ? ($('#chapter-description-en').val().length > 75) ? ': ' + $('#chapter-description-en').val().substr(0,75) + '...' : ': ' + $('#chapter-description-en').val() : '';
          var form_data = $('#chapter-form').serialize();
          $('#taxon-description-chapter-newform').html('<div class="messages-throbber">' + Drupal.t('Saving...') + '</div>');
          $.post(Drupal.settings.chapter_config_callback + "add_chapter/", form_data, function(data) {
            $('#hidden').clone().insertAfter($('#tid_' + pid)).attr('id','tid_' + data.term.tid);
            $('#tid_' + data.term.tid).removeClass("ui-state-hidden").addClass("ui-state-default").find('.editable').html('<strong>' + data.term.name + '</strong>' + new_description);
            $('#tid_' + data.term.tid + ' div:eq(2)').attr('id','removeme-' + data.term.tid);
            $('#tid_' + data.term.tid + ' div:eq(3)').attr('id','editme-' + data.term.tid);
            $('#tid_' + data.term.tid).fadeIn("slow");
            _this.hideMessage();
          }, "json");
      }
    });

  };

  TD.editChapter = function(ele) {
      var _this = this;
      var tid = ele.parent().attr('id').replace('editme-','');
      var tval = ele.parent().parent().children('div:eq(1)');

      var list_item = ele.parent().parent();
      var list_item_height = ele.parent().parent().height();
      var list_item_html = ele.parent().parent().html();
      list_item.addClass("ui-state-disabled").addClass("ui-state-form").animate({height: "225px"},1000).find("div").hide();
      list_item.html('<div class="messages-throbber">' + Drupal.t('Loading...') + '</div>');

      //get the stuff
      $.post(Drupal.settings.chapter_config_callback + "get_chapter/", {"tid" : tid}, function(result) {

        //make the form to post
        var chapter_form = '<form id="chapter-form">';
        chapter_form += '<div id="taxon-description-chapter-editform">';
        chapter_form += '<div id="taxon-description-close-editform" class="taxon-description-close"><a href="#" onclick="return false;">' + Drupal.t('close') + '</a></div>';
        chapter_form += '<fieldset><legend>' + Drupal.t('Edit') + ' ' + result.data['en'].name + '</legend><div id="taxon-description-editchapter-wrapper">';

        var languages = Drupal.settings.lang;

        $.each(languages, function(key, value) {

          var chapter_title, chapter_description, chapter_tid, chapter_parent, chapter_weight, chapter_vid, chapter_language, chapter_trid;

              if(result.data[key]!==undefined) {
            chapter_title = result.data[key].name;
            chapter_description = result.data[key].description;
                chapter_tid = result.data[key].tid;
                chapter_parent = result.data[key].parent;
                chapter_weight = result.data[key].weight;
                chapter_vid = result.data[key].vid;
                chapter_language = result.data[key].language;
                chapter_trid = result.data[key].trid;
          }
          else {
            chapter_title = "";//result.data['en'].name;
            chapter_description = "";// result.data['en'].description;
                chapter_tid = "";//result.data['en'].tid;
                chapter_parent = "";//result.data['en'].parent;
                chapter_weight = "";//result.data['en'].weight;
                chapter_vid = "";//result.data['en'].vid;
                chapter_language = "";//result.data['en'].language;
                chapter_trid = "";//result.data['en'].trid;
          }

          chapter_form += '<fieldset class= "collapsible collapsed"><legend>'+value+'</legend><div>';

          chapter_form += '<div id="chapter-name-wrapper" class="form-item">';
          chapter_form += '<label for="chapter-name-'+key+'">' + Drupal.t('Chapter') + ': <span class="form-required" title="' + Drupal.t('This field is required.') + '">*</span></label>';
          chapter_form += '<input id="chapter-name-'+key+'" class="form-text required" size="60" maxlength="100" type="text" name="taxon-description['+key+'][name]" value="' + chapter_title + '"></input>';
          chapter_form += '</div>';

          chapter_form += '<div id="chapter-description-wrapper" class="form-item">';
          chapter_form += '<label for="chapter-description-'+key+'">' + Drupal.t('Description') + '</label>';
          chapter_form += '<textarea id="chapter-description-'+key+'" class="form-textarea" rows="5" cols="60" name="taxon-description['+key+'][description]">' + chapter_description + '</textarea>';
          chapter_form += '</div>';

          chapter_form += '</div></fieldset>';

              chapter_form += '<input type="hidden" name="taxon-description['+key+'][tid]" value=' + chapter_tid + '></input>';
          chapter_form += '<input type="hidden" name="taxon-description['+key+'][parent]" value=' + chapter_parent + '></input>';
              chapter_form += '<input type="hidden" name="taxon-description['+key+'][weight]" value=' + chapter_weight + '></input>';
              chapter_form += '<input type="hidden" name="taxon-description['+key+'][vid]" value=' + chapter_vid + '></input>';
              chapter_form += '<input type="hidden" name="taxon-description['+key+'][language]" value=' + chapter_language + '></input>';
              chapter_form += '<input type="hidden" name="taxon-description['+key+'][trid]" value=' + chapter_trid + '></input>';

        });

        chapter_form += '</div></fieldset>';

            chapter_form += '<input type="submit" id="taxon-description-submit" class="form-submit" value="' + Drupal.t('Save') + '" onclick="javascript:return false;">';
        chapter_form += '<a id="taxon-description-cancel">' + Drupal.t('Cancel') + '</a>';
        chapter_form += '</div>';
        chapter_form += '</form>';

        list_item.html(chapter_form);

        $('#taxon-description-submit').click(function() {
          var valid = true;
          $('#taxon-description-editchapter-wrapper .required').each(function() {
            if($(this).val() == "" || $(this).val() == Drupal.t("Undefined")) {
              valid = false;
              Drupal.toggleFieldset($(this).parent().parent().parent().parent().addClass('collapsed')[0]);
              $(this).addClass("error").keyup(function() { $(this).removeClass("error"); });
            }
           });
           if(valid) {
             var new_name = $('#chapter-name-en').val();
             var new_description = ($('#chapter-description-en').val()) ? ($('#chapter-description-en').val().length > 75) ? ': ' + $('#chapter-description-en').val().substr(0,75) + '...' : ': ' + $('#chapter-description-en').val() : '';
              var form_data = $('#chapter-form').serialize();
              list_item.html('<div class="messages-throbber">' + Drupal.t('Saving...') + '</div>');
              $.post(Drupal.settings.chapter_config_callback + "update_chapter/", form_data, function(data) {
                list_item.removeClass("ui-state-disabled").removeClass("ui-state-form").html(list_item_html).animate({height: list_item_height},1000);
                list_item.find('.editable').html('<strong>' + new_name + '</strong>' + new_description);
              },"json");
           }  
        });

        $('#taxon-description-cancel').click(function() {
          list_item.removeClass("ui-state-disabled").removeClass("ui-state-form").html(list_item_html).animate({height: list_item_height},1000);
        });

        $('.taxon-description-close a').click(function() {
          list_item.removeClass("ui-state-disabled").removeClass("ui-state-form").html(list_item_html).animate({height: list_item_height},1000);
        });
        Drupal.attachBehaviors($('#taxon-description-editchapter-wrapper'));
      }, "json");
  };

  TD.deleteChapter = function(ele) {
      var _this = this;
      var tid = ele.parent().attr('id').replace('removeme-','');
      var chapter = ele.parent().parent().children('div:eq(1)').find("strong").text();
      var chapter_element = $('#removeme-'+tid);
      $.post(Drupal.settings.chapter_config_callback + "check_chapter_content/", {"tid" : tid, "name" : chapter}, function(data) {
        var message = '<div class="taxon-description-warning messages warning">';
        message += data.message + '<br /><br />';
        message += '<input type="submit" id="edit-delete" class="form-submit" value="' + Drupal.t('Delete') + '">';
        message += '  <a id="taxon-description-cancel">' + Drupal.t('Cancel') + '</a>';
        message += '</div>';
        _this.showMessage(message);
        $('#edit-delete').click(function() {
            $.post(Drupal.settings.chapter_config_callback + "delete_chapter/", {"tid" : tid }, function(data) { _this.hideMessage(); }, "json");
            chapter_element.parent().fadeOut("slow");
        });
        $('#taxon-description-cancel').click(function() {
          _this.hideMessage();
        });
      }, "json");
  };

  TD.displayMessage = function(message) {

  };

  TD.showMessage = function(message) {
      $('body').append('<div id="taxon-description-overlay"></div><div id="taxon-description-message"></div>');
      var arrPageSizes = ___getPageSize();
      $('#taxon-description-overlay').css({
        backgroundColor: 'black',
        opacity: 0.66,
        width: arrPageSizes[0],
        height: arrPageSizes[1]
      });
      var arrPageScroll = ___getPageScroll();
      $('#taxon-description-message').css({
        top: 50,
        left: arrPageScroll[0],
        position: 'fixed',
        zIndex: 1001,
        margin: '0px auto',
        width: '100%'
      });
      $('#taxon-description-overlay').show();
      $('#taxon-description-message').html(message).show();
  };

  TD.hideMessage = function() {
      $('#taxon-description-message').hide();
      $('#taxon-description-overlay').hide();
  };

});

$(window).resize(function() {
    // Get page sizes
    var arrPageSizes = ___getPageSize();
    // Style overlay and show it
    $('#taxon-description-overlay').css({
        width: arrPageSizes[0],
        height: arrPageSizes[1]
    });
    var arrPageScroll = ___getPageScroll();
    $('#taxon-description-message').css({
        top: arrPageScroll[1] + (arrPageSizes[3] / 3.5),
        left: arrPageScroll[0],
        position: 'fixed',
        zIndex: 1001,
        margin: '0px auto',
        width: '100%'
    });
});