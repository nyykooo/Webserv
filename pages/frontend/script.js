
function expandMenu() {
  document.getElementById("sidebar").classList.toggle("active");
  document.getElementById("overlay").classList.toggle("active");
}
function expandSubMenu(className) {
  console.log(className);
  const elements = document.getElementsByClassName(className);
  for (let element of elements) {
    if (element.classList.contains("active")) {
      element.classList.remove("active");
      element.classList.add("inactive");
    } else {
      element.classList.remove("inactive");
      element.classList.add("active");
    }
  }
}

function goToPage(string)
{
	console.log(string);
}