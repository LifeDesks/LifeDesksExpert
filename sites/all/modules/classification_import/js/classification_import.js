var classification_import_counter = 0;

function classification_import() {
  return {
    import_status: function(content) {
	  var _this = this;
      switch(content.status) {
    	  case "success":
          if(content.curr_count) {
            $("#import_status").html('<div class="messages status" id="import_message">Count: ' + content.curr_count + ' of ' + content.tot_count + ' (' + parseInt((parseInt(content.curr_count)/parseInt(content.tot_count))*100) + '%) Name: ' + content.name + '</div>');
            if(parseInt(content.curr_count) < parseInt(content.tot_count)) {
		       $.get(Drupal.settings.basePath + "classification_import/import_status/" + content.tot_count,{  },function(data) { _this.import_status(data); }, "json");
            }
            else if (parseInt(content.curr_count)+1 >= parseInt(content.tot_count)) {
               $.post(Drupal.settings.basePath + "classification_import/import_message/",{ "tot_count" : content.tot_count, "vid" : content.vid, "status" : content.status},function(data) { _this.import_message(data); }, "json");
            }
            else {}
          }
          else {
	        classification_import_counter++;
	        if(classification_import_counter < 5) {
		      $.get(Drupal.settings.basePath + "classification_import/import_status/" + content.tot_count,{  },function(data) { _this.import_status(data); }, "json");
	        }
	        else {
		      $.post(Drupal.settings.basePath + "classification_import/import_message/",{ "tot_count" : content.tot_count, "vid" : content.vid, "status" : content.status},function(data) { _this.import_message(data); }, "json");
	        }
	        
          }
          break;
        case "fail":
             $.post(Drupal.settings.basePath + "classification_import/import_message/",{ "tot_count" : content.tot_count, "vid" : content.vid, "status" : content.status},function(data) { _this.import_message(data); }, "json");
          break;
        default:
     }
    },
	import_message: function(content){      
      $("#import_status").html(content.message);     
	}
  }
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

$(function(){

    //disable form submission
	$("#edit-taxonomic-name").keyup(function(e) { 
       $("form").submit(function() {
	     $('#import_status').html('');
	     return false; 
	   });
	});
		
	var IMPORT = new classification_import();
	var search_callback = Drupal.settings.basePath + "classification_import/search/";

    //override the ActiveX jQuery settings
    $.ajaxSetup({
	  xhr:function() { return new XMLHttpRequest(); }
    });
		
    var search_results = $('#search_results');

	$('#edit-search').click(function() {
	  var vid = $("input[name='vid']:checked").val();
	  if(vid < 1) {
		$('.classification-selector input').addClass('error');
	    return;	
	  }
	  $('#search_status').show();
	  search_results.html('');
	  var name = $("#edit-taxonomic-name").val();
      $.post(search_callback,{ "search" : name }, function(data) {
	    if(data.status == "ok") {
		  if(!data.data) {
			search_results.html('<div class="messages error" id="import_message">' + data.message + '</div>');
			$('#search_status').hide();
		    return;	
		  }
		  var message = data.message;
		  var classifications = data.data.value;
		  var output = '';
		  output += '<div class="form-item" id="classification_search_results">';
		  output += '<label>' + message + '</label>';
		  output += '<div class="form-radios">';
		  if(classifications.length>1) {
		    for(i=0;i<=classifications.length-1;i++) {
			  output += '<div class="form-item">';
			  output += '<input type="radio" class="form-radio classifications_options" id="classification_' + i + '" name="classification_search_results" value="' + classifications[i].id + '">';
			  output += classifications[i].metadata.title;
			  output += (classifications[i].metadata.url.length > 0) ? ' <a href="' + classifications[i].metadata.url + '" target="_blank">' + Drupal.t('link') + '</a>' : '';
              output += '<span class="search_num_children">' + Drupal.t("Number of taxa at or under !name that will be imported", { "!name" : classifications[i].name }) + ': ' + ((classifications[i].number_of_children.length > 0) ? classifications[i].number_of_children : 0) + '</span>';
              output += '<span class="search_ancestry">' + ((classifications[i].ancestry.length > 0) ? classifications[i].ancestry.replace(/\|/g," | ") : "") + '</span>';
			  output += '</input>';
			  output += '</div>';
			  search_results.data('children_' + i,classifications[i].number_of_children);
		    }
		  }
		  else {
			 output += '<div class="form-item">';
			 output += '<input type="radio" class="form-radio classifications_options" id="classification_0" name="classification_search_results" value="' + classifications.id + '">';
			 output += classifications.metadata.title;
			 output += (classifications.metadata.url.length > 0) ? ' <a href="' + classifications.metadata.url + '" target="_blank">' + Drupal.t('link') + '</a>' : '';
             output += '<span class="search_num_children">' + Drupal.t("Number of taxa at or under !name that will be imported", { "!name" : classifications.name }) + ': ' + ((classifications.number_of_children.length > 0) ? classifications.number_of_children : 0) + '</span>';
             output += '<span class="search_ancestry">' + ((classifications.ancestry.length > 0) ? classifications.ancestry.replace(/\|/g," | ") : "") + '</span>';
			 output += '</input>';
			 output += '</div>';
			 search_results.data('children_0',classifications.number_of_children);
		  }
		  output += '</div>';
		  output += '</div>';
		  search_results.html(output);
	    }
	    else {
		  var message = Drupal.t('Sorry, the search service is currently unavailable');
		  search_results.html('<div class="messages error" id="import_message">' + message + '</div>');
	    }
	    $('#search_status').hide();
	    $.scrollTo('#classification_search_results', 800);
	
	    var numclass = $('.classifications_options').length;
	    for(j=0;j<=numclass-1;j++) {
	      $('#classification_' + j).data('children',search_results.data('children_' + j));
	      $('#classification_' + j).click(function() {
		     $('#import_message').remove();
		     var children = $(this).data('children');
		     if(children > parseInt(Drupal.settings.classification_import_tcs_limit)) {
			   $('#edit-eol-import').remove();
			   alert(Drupal.t('Sorry, there is a limit of !limit names. Please narrow your search or choose another classification.', {'!limit' : addCommas(parseInt(Drupal.settings.classification_import_tcs_limit))}));	
		     }
		     else {
			   $('#edit-eol-import').remove();
			   search_results.append('<input type="submit" id="edit-eol-import" class="form-submit" value="' + Drupal.t('Import') + '" name="op">');
			   $('#edit-eol-import').click(function() {
			     var val = $('input[name=classification_search_results]:checked').val();
			     $('#edit-eol-import').remove();
			     $.post(Drupal.settings.basePath + "classification_import/import_classification/", { "tid" : val, "vid" : vid, "tot_count" : children });
			     $.get(Drupal.settings.basePath + "classification_import/import_status/" + children,{ },function(data) {IMPORT.import_status(data);}, "json");
			   });
		     }
		  });
	    }
      },"json");
    });		
});