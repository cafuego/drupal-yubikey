// $Id: yubikey.js,v 1.2 2008/12/31 07:06:44 rubinj Exp $

Drupal.behaviors.yubikey = function (context) {
  var $loginElements   = $("#edit-name-wrapper, #edit-pass-wrapper, li.openid-link, li.yubikey-link");
  var $openidElements  = $("#edit-openid-identifier-wrapper, li.user-link");
  var $yubikeyElements = $("#edit-yubikey-otp-wrapper, li.yubikey-user-link");
  var $yubikeyLoginLink = $("li.yubikey-link");

  // This behavior attaches by ID, so is only valid once on a page.
  if(!$("#edit-yubikey-otp.yubikey-processed").size() && $("#edit-yubikey-otp.error").length) {
    $("#edit-yubikey-otp").addClass('yubikey-processed');
    $loginElements.hide();
    // Use .css("display", "block") instead of .show() to be Konqueror friendly.
    $yubikeyElements.css("display", "block");
  }

  $("li.openid-link:not(.yubikey-openid-processed)", context).addClass('yubikey-openid-processed').click(function() {
    // click openid link
    $yubikeyElements.hide();
    $yubikeyLoginLink.hide();
  });

  $("li.user-link:not(.yubikey-openid-processed)", context).addClass('yubikey-openid-processed').click(function() {
    // click cancel openid link
    $yubikeyElements.hide();
    $yubikeyLoginLink.show();
  });

  $("li.yubikey-link:not(.yubikey-processed)", context).addClass('yubikey-processed').click(function() {
    // click yubikey link
    $loginElements.hide();
    $openidElements.hide();
    $yubikeyElements.css("display", "block");
    // Remove possible error message.
    $("#edit-name, #edit-pass").removeClass("error");
    $("div.messages.error").hide();
    // Set focus on YubiKey Identifier field.
    $("#edit-yubikey-otp")[0].focus();
    return false;
  });

  $("li.yubikey-user-link:not(.yubikey-processed)", context).addClass('yubikey-processed').click(function() {
    // click cancel yubikey link
    $openidElements.hide();
    $yubikeyElements.hide();
    $loginElements.css("display", "block");
    // Clear YubiKey Identifier field and remove possible error message.
    $("#edit-yubikey-otp").val('').removeClass("error");
    $("div.messages.error").css("display", "block");
    // Set focus on username field.
    $("#edit-name")[0].focus();
    return false;
  });
};
