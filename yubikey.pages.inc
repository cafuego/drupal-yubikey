<?php

// $Id: yubikey.pages.inc,v 1.1.2.1 2009/02/04 06:23:35 rubinj Exp $

/*
 * @file
 * User page callbacks for the openid module.
 */

function yubikey_user_identities($account) {
  drupal_set_title(check_plain($account->name));
  drupal_add_css(drupal_get_path('module', 'yubikey') .'/yubikey.css', 'module');

  $header = array(t('YubiKey'), t('Operations'));
  $rows = array();

  $result = db_query("SELECT * FROM {authmap} WHERE module='yubikey' AND uid=%d", $account->uid);
  while ($identity = db_fetch_object($result)) {
    $rows[] = array(check_plain($identity->authname), l(t('Delete'), 'user/'. $account->uid .'/yubikey/delete/'. $identity->aid));
  }

  $output  = drupal_get_form('yubikey_user_pass');
  $output .= theme('table', $header, $rows);
  $output .= drupal_get_form('yubikey_user_add');
  return $output;
}

function yubikey_user_pass() {
  $form['require_pass'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require password in addition to YubiKey?'),
    '#default_value' => variable_get('yubikey_reqpass_'. arg(1), TRUE)
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Update YubiKey Password Settings'));
  return $form;
}

function yubikey_user_pass_validate($form, &$form_state) {
  $reqpass = $form_state['values']['require_pass'];
  variable_set('yubikey_reqpass_'. arg(1), $reqpass);
  if ($reqpass) {
    drupal_set_message(t('Password will be required when logging in with YubiKey'));
  }
  else {
    drupal_set_message(t('Password will not be required when logging in with YubiKey'));
  }
}

function yubikey_user_add() {
  $form['yubikey_otp'] = array(
    '#type' => 'password',
    '#required' => TRUE,
    '#title' => t('YubiKey')
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Add a YubiKey'));
  return $form;
}

function yubikey_user_add_validate($form, &$form_state) {
  module_load_include('inc', 'yubikey');
  $otp = $form_state['values']['yubikey_otp'];
  $key_id = yubikey_keyid($otp);
  if (db_result(db_query("SELECT authname FROM {authmap} WHERE authname='%s'", $key_id))) {
    form_set_error('yubikey_otp', t('That YubiKey is already in use on this site.'));
  }
  else {
    if (yubikey_validate($otp, $errstr)) {
      db_query("INSERT INTO {authmap} (uid, authname, module) VALUES (%d, '%s','yubikey')", arg(1), $key_id);
      drupal_set_message(t('Successfully added YubiKey ID %identity', array('%identity' => $key_id)));
    }
    else {
      form_set_error('yubikey_otp', t('Sorry, YubiKey OTP validation failed with message:') .' '. $errstr);
    }
  }
}

function yubikey_user_delete_form($form_state, $account, $aid = 0) {
  $key_id = db_result(db_query('SELECT authname FROM {authmap} WHERE uid = %d AND aid = %d', $account->uid, $aid));
  $form = array();
  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $account->uid
  );
  $form['aid'] = array(
    '#type' => 'value',
    '#value' => $aid,
  );
  return confirm_form($form, t('Are you sure you want to delete the YubiKey %key_id for %user?', array('%key_id' => $key_id, '%user' => $account->name)), 'user/'. $account->uid .'/yubikey');
}

function yubikey_user_delete_form_submit($form, &$form_state) {
  db_query("DELETE FROM {authmap} WHERE uid = %d AND aid = %d AND module = 'yubikey'", $form_state['values']['uid'], $form_state['values']['aid']);
  if (db_affected_rows()) {
    drupal_set_message(t('YubiKey deleted.'));
  }
  $form_state['redirect'] = 'user/'. $form_state['values']['uid'] .'/yubikey';
}

function yubikey_password_page() {
  $name = $_SESSION['yubikey']['name'];
  if (!isset($name)) {
    return drupal_access_denied();
  }
  $form['pass'] = array(
    '#type' => 'password',
    '#required' => TRUE,
    '#title' => t('Password'),
    '#size' => 60,
    '#maxlength' => 128
  );
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $name
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Log in'));
  return $form;
}

function yubikey_password_page_validate($form, &$form_state) {
  if (user_authenticate($form_state['values']) && !form_get_errors()) {
    $return_to = $_SESSION['yubikey']['return_to'];
    unset($_SESSION['yubikey']['name']);
    drupal_goto();
  }
  else {
    form_set_error('pass', t('Invalid Password'));
  }
}

function yubikey_admin_page() {
  drupal_add_js(drupal_get_path('module', 'yubikey') .'/yubikey_admin.js', 'module');
  $form['yubikey'] = array(
    '#type'  => 'fieldset',
    '#title' => t('YubiKey Sitewide Settings'),
    '#description' => t('You can get your API Key and API ID from') .' '. l(t('Yubico'), 'https://upgrade.yubico.com/getapikey/') .'.',
    '#tree'  => TRUE
  );
  $form['yubikey']['enable'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable Yubikey Authentication'),
    '#default_value' => variable_get('yubikey_enabled', FALSE)
  );
  $form['yubikey']['apikey'] = array(
    '#type'          => 'textfield',
    '#title'         => t('API Key'),
    '#default_value' => variable_get('yubikey_apikey', NULL),
    '#size'          => 32,
    '#maxlength'     => 128,
    '#required'      => TRUE,
    '#prefix'        => '<div id="api_fields_div">'
  );
  $form['yubikey']['apiid'] = array(
    '#type'          => 'textfield',
    '#title'         => t('API ID'),
    '#default_value' => variable_get('yubikey_apiid', NULL),
    '#size'          => 6,
    '#maxlength'     => 128,
    '#required'      => TRUE
  );
  $form['yubikey']['tolerance'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Time Stamp Tolerance'),
    '#description'   => t('How many seconds apart the YubiKey API server response may be from your Drupal server\'s time.') .'<br />'. t('If this is too small and your clock is incorrect, your authentication requests may fail.'),
    '#default_value' => variable_get('yubikey_tolerance', 150),
    '#size'          => 6,
    '#maxlength'     => 128
  );
  $form['yubikey']['timeout'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Timeout'),
    '#description'   => t('How long to wait (in seconds) for an authentication response before the request is considered a failure.'),
    '#default_value' => variable_get('yubikey_timeout', 10),
    '#size'          => 6,
    '#maxlength'     => 128,
    '#suffix'        => '</div>'
  );
  $form['#submit'][] = 'yubikey_admin_page_submit';
  return system_settings_form($form);
}

function yubikey_admin_page_submit($form_id, &$form_state) {
  if ($form_state['values']['op'] == t('Reset to defaults')) {
    variable_del('yubikey_enabled');
    variable_del('yubikey_apikey');
    variable_del('yubikey_apiid');
    variable_del('yubikey_tolerance');
    variable_del('yubikey_timeout');
  }
  else {
    $enabled   = $form_state['values']['yubikey']['enable'];
    $apikey    = $form_state['values']['yubikey']['apikey'];
    $apiid     = $form_state['values']['yubikey']['apiid'];
    $tolerance = $form_state['values']['yubikey']['tolerance'];
    $timeout   = $form_state['values']['yubikey']['timeout'];
    variable_set('yubikey_enabled',   $enabled);
    variable_set('yubikey_apikey',    $apikey);
    variable_set('yubikey_apiid',     $apiid);
    variable_set('yubikey_tolerance', $tolerance);
    variable_set('yubikey_timeout',   $timeout);
  }
}
