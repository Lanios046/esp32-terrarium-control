const ROOT_URL = window.location.href;

var versionString = "";

const PAGES = Object.freeze({
    LOGIN:     0,
    WIFI:      1,
    ACCOUNT:   2,
	INTERVALS: 3,
	SYSTEM:    4,
	FACTORY:   5
});

function get_version_string() {
	get_json("api/settings/name_version").then(response => {
		versionString = response["name_version"];
	}).catch(error => {
		show_toast(error.message);
	});
}

function swap_main_password_visible() {
	var passwordEdit = document.getElementById("LSPasswordArea");
	var passwordEditEye = document.getElementById("LSPasswordEye");
	if (passwordEdit.type === "password") {
		passwordEdit.type = "text";
		passwordEditEye.src = "eye-slash.png";
	} else {
		passwordEdit.type = "password";
		passwordEditEye.src = "eye.png";
	}
}

function swap_wifi_password_visible() {
	var passwordEdit = document.getElementById("WiFiPasswordArea");
	var passwordEditEye = document.getElementById("WiFiPasswordEye");
	if (passwordEdit.type === "password") {
		passwordEdit.type = "text";
		passwordEditEye.src = "eye-slash.png";
	} else {
		passwordEdit.type = "password";
		passwordEditEye.src = "eye.png";
	}
}

function swap_account_password_visible() {
	var passwordEdit = document.getElementById("AccPasswordArea");
	var passwordEditEye = document.getElementById("AccPasswordEye");
	if (passwordEdit.type === "password") {
		passwordEdit.type = "text";
		passwordEditEye.src = "eye-slash.png";
	} else {
		passwordEdit.type = "password";
		passwordEditEye.src = "eye.png";
	}
}

function swap_system_password_visible() {
	var passwordEdit = document.getElementById("SysPasswordArea");
	var passwordEditEye = document.getElementById("SysPasswordEye");
	if (passwordEdit.type === "password") {
		passwordEdit.type = "text";
		passwordEditEye.src = "eye-slash.png";
	} else {
		passwordEdit.type = "password";
		passwordEditEye.src = "eye.png";
	}
}

function force_open_screen_debug(page) {
	switch (page)
	{
		case PAGES.LOGIN:
			document.getElementById('LOGINSCREEN').style.visibility = 'visible';
			document.getElementById('MAINMENU').style.visibility = 'hidden';
			document.getElementById('WIFISCREEN').style.visibility = 'hidden';
			document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
			document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
			document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
			document.getElementById('FACTORY').style.visibility = 'hidden';
			break;

		case PAGES.WIFI:
			document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
			document.getElementById('MAINMENU').style.visibility = 'visible';
			document.getElementById('WIFISCREEN').style.visibility = 'visible';
			document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
			document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
			document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
			document.getElementById('FACTORY').style.visibility = 'hidden';
			get_wifi_settings();
			break;

		case PAGES.ACCOUNT:
			document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
			document.getElementById('MAINMENU').style.visibility = 'visible';
			document.getElementById('WIFISCREEN').style.visibility = 'hidden';
			document.getElementById('ACCOUNTSCREEN').style.visibility = 'visible';
			document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
			document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
			document.getElementById('FACTORY').style.visibility = 'hidden';
			get_account_settings();
			break;

		case PAGES.INTERVALS:
			document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
			document.getElementById('MAINMENU').style.visibility = 'visible';
			document.getElementById('WIFISCREEN').style.visibility = 'hidden';
			document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
			document.getElementById('INTERVALSSCREEN').style.visibility = 'visible';
			document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
			document.getElementById('FACTORY').style.visibility = 'hidden';
			get_intervals_settings();
		break;

		case PAGES.SYSTEM:
			document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
			document.getElementById('MAINMENU').style.visibility = 'visible';
			document.getElementById('WIFISCREEN').style.visibility = 'hidden';
			document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
			document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
			document.getElementById('SYSTEMSCREEN').style.visibility = 'visible';
			document.getElementById('FACTORY').style.visibility = 'hidden';
			get_device_settings();
		break;

		case PAGES.FACTORY:
			document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
			document.getElementById('MAINMENU').style.visibility = 'hidden';
			document.getElementById('WIFISCREEN').style.visibility = 'hidden';
			document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
			document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
			document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
			document.getElementById('FACTORY').style.visibility = 'visible';
		break;

		default:
			break;
	}
}

function force_open_login_screen() {
	document.getElementById('LOGINSCREEN').style.visibility = 'visible';
	document.getElementById('MAINMENU').style.visibility = 'hidden';
	document.getElementById('WIFISCREEN').style.visibility = 'hidden';
	document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
	document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
	document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
	document.getElementById('FACTORY').style.visibility = 'hidden';
}

function open_login_screen() {
	let json = {page: PAGES.LOGIN};
	send_json("api/settings/page", JSON.stringify(json));
	update_state();
}

function open_wifi_screen() {
	let json = {page: PAGES.WIFI};
	send_json("api/settings/page", JSON.stringify(json));
	update_state();
}

function open_account_screen() {
	let json = {page: PAGES.ACCOUNT};
	send_json("api/settings/page", JSON.stringify(json));
	update_state();
}
function open_intervals_screen() {
	let json = {page: PAGES.INTERVALS};
	send_json("api/settings/page", JSON.stringify(json));
	update_state();
}

function open_system_screen() {
	let json = {page: PAGES.SYSTEM};
	send_json("api/settings/page", JSON.stringify(json));
	update_state();
}

function update_state() {
	get_json("api/settings/page").then(response => {
		switch (response["page"])
		{
			case PAGES.LOGIN:
				document.getElementById('LOGINSCREEN').style.visibility = 'visible';
				document.getElementById('MAINMENU').style.visibility = 'hidden';
				document.getElementById('WIFISCREEN').style.visibility = 'hidden';
				document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
				document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
				document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
				document.getElementById('FACTORY').style.visibility = 'hidden';
				break;
	
			case PAGES.WIFI:
				document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
				document.getElementById('MAINMENU').style.visibility = 'visible';
				document.getElementById('WIFISCREEN').style.visibility = 'visible';
				document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
				document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
				document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
				document.getElementById('FACTORY').style.visibility = 'hidden';
				get_wifi_settings();
				break;
	
			case PAGES.ACCOUNT:
				document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
				document.getElementById('MAINMENU').style.visibility = 'visible';
				document.getElementById('WIFISCREEN').style.visibility = 'hidden';
				document.getElementById('ACCOUNTSCREEN').style.visibility = 'visible';
				document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
				document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
				document.getElementById('FACTORY').style.visibility = 'hidden';
				get_account_settings();
				break;

			case PAGES.INTERVALS:
				document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
				document.getElementById('MAINMENU').style.visibility = 'visible';
				document.getElementById('WIFISCREEN').style.visibility = 'hidden';
				document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
				document.getElementById('INTERVALSSCREEN').style.visibility = 'visible';
				document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
				document.getElementById('FACTORY').style.visibility = 'hidden';
				get_intervals_settings();
			break;
	
			case PAGES.SYSTEM:
				document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
				document.getElementById('MAINMENU').style.visibility = 'visible';
				document.getElementById('WIFISCREEN').style.visibility = 'hidden';
				document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
				document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
				document.getElementById('SYSTEMSCREEN').style.visibility = 'visible';
				document.getElementById('FACTORY').style.visibility = 'hidden';
				get_device_settings();
			break;

			case PAGES.FACTORY:
				document.getElementById('LOGINSCREEN').style.visibility = 'hidden';
				document.getElementById('MAINMENU').style.visibility = 'hidden';
				document.getElementById('WIFISCREEN').style.visibility = 'hidden';
				document.getElementById('ACCOUNTSCREEN').style.visibility = 'hidden';
				document.getElementById('INTERVALSSCREEN').style.visibility = 'hidden';
				document.getElementById('SYSTEMSCREEN').style.visibility = 'hidden';
				document.getElementById('FACTORY').style.visibility = 'visible';
			break;
	
			default:
				break;
		}
	}).catch(error => {
		show_toast(error.message);
	});

	const versionTitles = document.getElementsByClassName("versionTextTitle");
	for (let i = 0; i < versionTitles.length; i++){
		versionTitles[i].textContent = versionString;
	}
}

function try_auth() {
    let json = {
        loginUsername:  document.getElementById('LSLoginArea').value,
        loginPassword:  document.getElementById('LSPasswordArea').value
    };

	let json_string = JSON.stringify(json);

	send_json("api/auth", json_string).then(send_response => {
		document.getElementById('LSLoginArea').value  = "";
		document.getElementById('LSPasswordArea').value  = "";
		show_toast("Отправка данных приостановлена");
	}).catch(error => {
		show_toast(error.message);
	});

}

function set_wifi_settings() {
	var ssidInputRegExp = new RegExp('[\\w \\s]\{3,32\}','g'); // 3 - 32 char
	var passInputRegExp = new RegExp('[\\S]\{8,32\}','g'); // 8 - 32 non whitespace char

	if (!ssidInputRegExp.test(document.getElementById('WiFiLoginArea').value.toString())
		|| !passInputRegExp.test(document.getElementById('WiFiPasswordArea').value.toString())){
			show_toast("Некорректный SSID или пароль");
			return;
	}

    let json = {
        ssid:      document.getElementById('WiFiLoginArea').value,
        password:  document.getElementById('WiFiPasswordArea').value
    };

	let json_string = JSON.stringify(json);

	show_toast(json_string);

	send_json("api/settings/wifi", json_string).then(send_response => {
		show_toast("Настройки будут применены после перезагрузки");
	}).catch(error => {
		show_toast(error.message);
	});
}

function set_wifi_settings_and_reboot() {
	var ssidInputRegExp = new RegExp('[\\w \\s]\{3,32\}','g'); // 3 - 32 char
	var passInputRegExp = new RegExp('[\\S]\{8,32\}','g'); // 8 - 32 non whitespace char

	if (!ssidInputRegExp.test(document.getElementById('WiFiLoginArea').value.toString())
		|| !passInputRegExp.test(document.getElementById('WiFiPasswordArea').value.toString())){
			show_toast("Некорректный SSID или пароль");
			return;
	}

    let json = {
        ssid:      document.getElementById('WiFiLoginArea').value,
        password:  document.getElementById('WiFiPasswordArea').value
    };

	let json_string = JSON.stringify(json);

	show_toast(json_string);

	send_json("api/settings/wifi", json_string).then(send_response => {
		show_toast("Выполняется перезапуск устройства");
		reboot();
	}).catch(error => {
		show_toast(error.message);
	});
}

function set_account_settings() {
	var emailInputRegExp = new RegExp('[a-zA-Z0-9._%+\-]+\@[a-z0-9.\-]+\.[a-z]\{2,\}\$','g'); // email regexp
	var passInputRegExp = new RegExp('[\\S]\{3,32\}','g'); // 3 - 32 non whitespace char

	if (!emailInputRegExp.test(document.getElementById('AccLoginArea').value.toString())
		|| !passInputRegExp.test(document.getElementById('AccPasswordArea').value.toString())){
			show_toast("Некорректный e-mail или пароль");
			return;
	}

    let json = {
        login:     document.getElementById('AccLoginArea').value,
        password:  document.getElementById('AccPasswordArea').value
    };

	let json_string = JSON.stringify(json);

	send_json("api/settings/account", json_string).then(send_response => {
		show_toast("Применено");
	}).catch(error => {
		show_toast(error.message);
	});
}

function set_intervals_settings() {
	var intervalMonitoringRegExp = new RegExp('\^[1-9][0-9]\{0,3\}\$','g'); // first digit not 0 + 3 any digits
	var intervalDataRegExp = new RegExp('\^[1-9][0-9]\{0,3\}\$','g'); // first digit not 0 + 3 any digits

	if (!intervalMonitoringRegExp.test(document.getElementById('IntervalMonitoringArea').value.toString())
		|| !intervalDataRegExp.test(document.getElementById('IntervalDataArea').value.toString())){
			show_toast("Некорректные значения периодов");
			return;
	}

    let json = {
        monitoring: document.getElementById('IntervalMonitoringArea').value,
        data:       document.getElementById('IntervalDataArea').value
    };

	let json_string = JSON.stringify(json);

	send_json("api/settings/intervals", json_string).then(send_response => {
		show_toast("Применено");
	}).catch(error => {
		show_toast(error.message);
	});
}

function set_device_settings() {
	var ssidInputRegExp = new RegExp('[\\w \\s]\{3,32\}','g'); // 3 - 32 char
	var passInputRegExp = new RegExp('[\\S]\{3,32\}','g'); // 3 - 32 non whitespace char

	if (!ssidInputRegExp.test(document.getElementById('SysLoginArea').value.toString())
		|| !passInputRegExp.test(document.getElementById('SysPasswordArea').value.toString())){
			show_toast("Некорректное имя пользователя или пароль");
			return;
	}

    let json = {
        login:     document.getElementById('SysLoginArea').value,
        password:  document.getElementById('SysPasswordArea').value
    };

	let json_string = JSON.stringify(json);

	send_json("api/settings/device", json_string).then(send_response => {
		show_toast("Применено");
	}).catch(error => {
		show_toast(error.message);
	});
}

function set_factory_settings() {
	var inputRegExp = new RegExp('[\\S]\{4,8\}','g'); // 4 - 8 digit

	if (!inputRegExp.test(document.getElementById('FactoryLoginArea').value.toString())){
			show_toast("Невалидный ID");
			return;
	}

    let json = {
        id:     document.getElementById('FactoryLoginArea').value,
    };

	let json_string = JSON.stringify(json);

	send_json("api/settings/service/factory", json_string).then(send_response => {
		show_toast("Применено");
	}).catch(error => {
		show_toast(error.message);
	});
}

function get_wifi_settings() {
	get_json("api/settings/wifi").then(response => {
		var loginIsEmpty = document.getElementById('WiFiLoginArea').value == "" || document.getElementById('WiFiLoginArea').value == null;
		var passwordIsEmpty = document.getElementById('WiFiPasswordArea').value == "" || document.getElementById('WiFiPasswordArea').value == null;
		if (loginIsEmpty && passwordIsEmpty){
			document.getElementById('WiFiLoginArea').value = response["ssid"];
			document.getElementById('WiFiPasswordArea').value = response["password"];
		}
	}).catch(error => {
		show_toast(error.message);
	});
}

function get_account_settings() {
	get_json("api/settings/account").then(response => {
		var loginIsEmpty = document.getElementById('AccLoginArea').value == "" || document.getElementById('AccLoginArea').value == null;
		var passwordIsEmpty = document.getElementById('AccPasswordArea').value == "" || document.getElementById('AccPasswordArea').value == null;
		if (loginIsEmpty && passwordIsEmpty){
			document.getElementById('AccLoginArea').value = response["login"];
			document.getElementById('AccPasswordArea').value = response["password"];
		}
	}).catch(error => {
		show_toast(error.message);
	});
}

function get_intervals_settings() {
	get_json("api/settings/intervals").then(response => {
		var monitoringIsEmpty = document.getElementById('IntervalMonitoringArea').value == "" || document.getElementById('IntervalMonitoringArea').value == null;
		var dataIsEmpty = document.getElementById('IntervalDataArea').value == "" || document.getElementById('IntervalDataArea').value == null;
		if (monitoringIsEmpty && dataIsEmpty){
			document.getElementById('IntervalMonitoringArea').value = response["monitoring"];
			document.getElementById('IntervalDataArea').value = response["data"];
		}
	}).catch(error => {
		show_toast(error.message);
	});
}

function get_device_settings() {
	get_json("api/settings/device").then(response => {
		var loginIsEmpty = document.getElementById('SysLoginArea').value == "" || document.getElementById('SysLoginArea').value == null;
		var passwordIsEmpty = document.getElementById('SysPasswordArea').value == "" || document.getElementById('SysPasswordArea').value == null;
		if (loginIsEmpty && passwordIsEmpty){
			document.getElementById('SysLoginArea').value = response["login"];
			document.getElementById('SysPasswordArea').value = response["password"];
		}
	}).catch(error => {
		show_toast(error.message);
	});
}

function reset_settings() {
	let json = {command: "reset"};
	send_json("api/settings/reset", JSON.stringify(json));
	get_device_settings();
}

function reboot() {
	let json = {command: "reboot"};
	send_json("api/settings/reboot", JSON.stringify(json));
	force_open_login_screen();
}

async function get_json(api_path) {
	let req_url = ROOT_URL + api_path;
	const response = await fetch(req_url, {
		method: 'GET',
		headers: {
			'Content-Type': 'application/json',
			'token': `${readCookie('token')}`
		},
	});

    const payload = await response.json();

    if (response.ok)
		return payload["message"];

    if (response.status == 500 && 'message' in payload)
		throw new Error(payload["message"]);

	throw new Error("Потеря соединения");
}

async function send_json(api_path, json_data = undefined) {
	let post_url = ROOT_URL + api_path;
	const response = await fetch(post_url, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json',
			'token': `${readCookie('token')}`
		},
		body: json_data
	});
    
    const payload = await response.json();

	if ("token" in payload){
		writeCookie('token', payload["token"]);
	}

    if (response.ok)
		return payload["message"];

    if (response.status == 500 && 'message' in payload)
		throw new Error(payload["message"]);

	throw new Error("Потеря соединения");
}

function readCookie(cookie)
{
  var myCookie = 'transponder'+"_"+cookie+"=";
  if (document.cookie) {
	var index = document.cookie.indexOf(myCookie);
	if (index != -1) {
	  var valStart = index + myCookie.length;
	  var valEnd = document.cookie.indexOf("; expires=", valStart);
	  if (valEnd == -1) {
		valEnd = document.cookie.length;
	  }
	  var val = document.cookie.substring(valStart, valEnd);
	  return val;
	}
  }
  return 0;
}

function writeCookie(cookie, val, expiration)
{
  if (val==undefined) return;
  if (expiration == null) {
	var date = new Date();
	date.setTime(date.getTime()+(10*365*24*60*60*1000)); // default expiration is one week
	expiration = date.toGMTString();
  }
  document.cookie = 'transponder' + "_" + cookie + "=" + val + "; expires=" + expiration+"; path=/";
}

function show_toast(msg) {
	Toastify({
		text: msg,
		duration: 3000,
		newWindow: true,
		close: true,
		gravity: "top", // `top` or `bottom`
		position: "center", // `left`, `center` or `right`
		backgroundColor: "#E50019",
		stopOnFocus: true, // Prevents dismissing of toast on hover
	}).showToast();
}