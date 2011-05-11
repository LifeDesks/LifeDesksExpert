var nameFormatter = function(row, cell, value, columnDef, dataContext) {
	if(dataContext.taxonomicrelationships) {
	  var relation = trim(dataContext.taxonomicrelationships.replace("name", ""));
	  return '<span class="' + relation + '">' + dataContext["name"] + '</span>';
	}
	else {
		return dataContext["name"];
	}
};

var nameDataHandler = function(value, columnDef, dataContext, extraData) {
	$.post(Drupal.settings.basePath + "classification_slickgrid_update_name", {"value" : value, "tid" : dataContext.tid, "vid" : columnDef.cssClass.replace("vid-","") }, function(data) {
		return {
            valid: true,
            msg: "Updated name"
        };
	}, "json");
};

var flagDataHandler = function(value, columnDef, dataContext, extraData) {
	$.post(Drupal.settings.basePath + "classification_slickgrid_update_flag", {"value" : value, "tid" : dataContext.tid, "flag_tid" : extraData.flag_tid, "flag_vid" : columnDef.cssClass.replace("vid-","") }, function(data) {
		return {
            valid: true,
            msg: "Updated or inserted new flag"
        };
	}, "json");
};

function trim(str) {
	str = str.replace(/^\s+/, '');
	for (var i = str.length - 1; i >= 0; i--) {
		if (/\S/.test(str.charAt(i))) {
			str = str.substring(0, i + 1);
			break;
		}
	}
	return str;
}
