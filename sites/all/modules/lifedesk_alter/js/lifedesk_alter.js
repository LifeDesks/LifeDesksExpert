$(function() {
    $('#edit-' + Drupal.settings.register_token).blur(function() {
       $('#edit-mail').val($(this).val()); 
    });
}); 