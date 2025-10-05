

function setCookie() {
    const name = document.getElementById("cookie-name");
    const value = document.getElementById("cookie-value");
    fetch(`/cookie_manager.py?action=set&name=${encodeURIComponent(name.value)}&value=${encodeURIComponent(value.value)}`)
    .then(response => response.text())
    .then(data => alert(data))
    .catch(error => {
        console.error("Error setting cookie:", error);
        alert("An error occurred while setting the cookie.");
    });
    name.value = "";
    value.value = "";
}

function getCookies() {
    fetch("/cookie_manager.py?action=get")
    .then(response => response.text())
    .then(data => document.getElementById("cookie-display").innerText = data)
    .catch(error => {
        console.error("Error setting cookie:", error);
        alert("An error occurred while getting cookies.");
    });
}

function deleteCookie() {
    const name = document.getElementById("cookie-name");
    fetch(`/cookie_manager.py?action=delete&name=${encodeURIComponent(name.value)}`)
    .then(response => response.text())
    .then(data => alert(data))
    .catch(error => {
        console.error("Error setting cookie:", error);
        alert("An error occurred while deleting the cookie.");
    });
    name.value = "";
}
