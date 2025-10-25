
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

// --- GET ---

async function testGET() {
	const file = document.getElementById('GET_file').value.trim();
	const statusDisplay = document.getElementById('test_response_status');

	statusDisplay.textContent = 'Loading...';

	try {
		const response = await fetch(file);

		statusDisplay.textContent = `Response HTTP status: ${response.status}`;

		let contentType = response.headers.get("content-type") || "";

		let result;
		if (contentType.includes("application/json")) {
			result = await response.json();
			console.log("JSON response:", result);
		} else {
			result = await response.text();
			console.log("Text response:", result);
		}
	} catch (error) {
		statusDisplay.textContent = `Error: ${error.message}`;
		console.error(error);
	}
}

// --- POST ---

const fileInput = document.getElementById('file');
const selectBtm = document.getElementById('upload_button');
const fileNameDisplay = document.getElementById('file_name');
const submitBtn = document.getElementById('submitBtn');
const message = document.getElementById('message');
const form = document.getElementById('file_upload_form');

selectBtm.addEventListener('click', function () {
	console.log("selected button clicked");
	selectBtm.click();
});

fileInput.addEventListener('change', function () {
	console.log("file input changed");
	if (fileInput.files.length > 0) {
		fileNameDisplay.textContent = fileInput.files[0].name;
	} else {
		fileNameDisplay.textContent = "No File Selected";
	}
});

async function testPOST(event) {
	event.preventDefault(); // prevent form from reloading page
	console.log("handling POST method");
  
	// Grab your file input and the first selected file
	const fileInput = document.getElementById("file");
	const statusDisplay = document.getElementById("test_response_status");
  
	if (!fileInput || fileInput.files.length === 0) {
	  statusDisplay.textContent = "Please select a file first.";
	  return;
	}
  
	const file = fileInput.files[0];
	console.log("File selected:", file.name);
  
	// Create a FormData object (browser will handle boundary + headers)
	const formData = new FormData();
	formData.append("file", file);
  
	try {
	  const response = await fetch("/", {
		method: "POST",
		body: formData, // the key part!
	  });
  
	  statusDisplay.textContent = `Response HTTP status: ${response.status}`;
  
	  const contentType = response.headers.get("content-type") || "";
	  let result;
  
	  if (contentType.includes("application/json")) {
		result = await response.json();
		console.log("JSON response:", result);
	  } else {
		result = await response.text();
		console.log("Text response:", result);
	  }
	} catch (error) {
	  console.error("Error:", error);
	  statusDisplay.textContent = `Error: ${error.message}`;
	}
  }
  