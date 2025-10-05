
const fileInput = document.getElementById('file');
const selectBtm = document.getElementById('uploadBtm');
const fileNameDisplay = document.getElementById('fileName');
const submitBtn = document.getElementById('submitBtn');
const message = document.getElementById('message');
const form = document.getElementById('file_upload_form');

selectBtm.addEventListener('click', function () {
    fileInput.click();
});

fileInput.addEventListener('change', function () {
    if (fileInput.files.length > 0) {
        fileNameDisplay.textContent = fileInput.files[0].name;
    } else {
        fileNameDisplay.textContent = "No File Selected";
    }
});

submitBtn.addEventListener('click', async (e) => {
    e.preventDefault();
    if (fileInput.files.length === 0) {
        message.style.display = 'block';
        message.innerHTML = "Please select a file before uploading";
        setTimeout( () => {
            message.style.display = 'none';
            message.innerHTML = "";
        }, 3000);
    }
    else {
        const name = fileInput.files[0].name;
        const file = fileInput.files[0];
        const form_data = new FormData(form);
        // form_data.append("name", name);
        // form_data.append("file", "INPUT");
        // console.log(e);
        // const form_data = e.currentTarget;
        // if (form_data === null) {
        //     throw new Error(`Invalid input`);
        // }
        // const url = new URL(form_data.action);
        // console.log(e.currentTarget);
        // console.log(new FormData(form_data));
        // console.log(e.currentTarget.action);
        const response = await fetch("/file_upload.py", {
            method: "POST",
            body: form_data
        })
        .then(response => {
            if (!response.ok) {
                throw new Error(`Response status: ${response.status}`);
            }
            return response.json();
        })
        .then(response => {
            message.style.display = "block";
            message.innerHTML = "File " + name + " saved successfully!";
            fileNameDisplay.textContent = "No File Selected";
            form.reset();
            setTimeout(() => {
                message.style.display = "none";
                message.innerHTML = "";
                console.log(response.message);
            }, 2000);
        })
        .catch(error => {
            console.error("Upload error:", error);
            message.innerHTML = "Upload failed. Please try again.";
            setTimeout(() => {
                message.style.display = "none";
                message.innerHTML = "";
                fileNameDisplay.textContent = "No File Selected";
            }, 3000);
        });
    }
});
