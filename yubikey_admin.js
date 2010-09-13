// $Id: yubikey_admin.js,v 1.2 2008/12/31 07:06:44 rubinj Exp $

$(document).ready(function() {
  if(!$("#edit-yubikey-enable").is(':checked')) {
    $('#api_fields_div').hide();
  }
  $('#edit-yubikey-enable').change(function() {
    if($(this).is(':checked')) {
      $('#api_fields_div').slideDown('fast');
    } else {
      $('#api_fields_div').slideUp('fast');
    }
  });
});
