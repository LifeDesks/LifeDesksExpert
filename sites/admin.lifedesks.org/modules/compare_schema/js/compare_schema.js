function updateGallery(nid) {
  $.post("/?q=admin/multisite/update_gallery/", {"nid" : nid }, function(data) {
	displayGalleryMessage(data,nid);
  }, "json");	
}

function updateSchema(nid) {
  $.post("/?q=admin/multisite/update/", {"nid" : nid }, function(data) {displayUpdateMessage(data,nid);}, "json");
}

function dropSessions(nid) {
  $.post("/?q=admin/multisite/drop_sessions/", {"nid" : nid }, function(data) {displayDropMessage(data,nid);}, "json");
}

function displayGalleryMessage(content,nid) {
  switch (content.status) {
    case "hidden":
      $('.compare_schema_gallery_nid_' + nid + ' a').text(Drupal.t('no'));
      break;
    case "shown":
      $('.compare_schema_gallery_nid_' + nid + ' a').text(Drupal.t('yes'));
      break;
  }
}

function displayUpdateMessage(content,nid) {
  var message_type = "";
  switch (content.status) {
    case "updated":
      message_type = "status";
      break;
    case "failed":
      message_type = "error";
      break;
    default:
      message_type = "";
  }
  if(content.status == 'updated'){
    $('.nid_' + nid).fadeOut("slow");
    $('#compare_schema_update_status_'+nid).attr('style','color:green').html('up to date');
  }
  else {
	$('#compare_schema_update_messages').html(content.data);
	$('#compare_schema_update_messages').fadeIn("fast");
  }
}

function displayDropMessage(content,nid) {
  var message_type = "";
  switch (content.status) {
    case "dropped":
      message_type = "status";
      break;
    case "failed":
      message_type = "error";
      break;
    default:
      message_type = "";
  }
  if(content.status == 'dropped'){
    $('.sessions_' + nid).fadeOut("slow");
    $('#compare_schema_sessions_'+nid).attr('style','color:green').html('0');
  }
  else {
	$('#compare_schema_update_messages').html(content.data);
	$('#compare_schema_update_messages').fadeIn("fast");
  }
}

function flushCache(url) {
	$.get('/flush_cache/' + url,{}, function(data) {
		if(data !== '"200"') {
			alert('Flush caches failed');
		}
		else {
			alert('Caches flushed for ' + url);
		}
	});
}

$(function() {
  //override the ActiveX jQuery settings
  $.ajaxSetup({
	xhr:function() { return new XMLHttpRequest(); }
  });
});