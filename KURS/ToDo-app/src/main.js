const input = document.querySelector("#input-box");
const button = document.querySelector("#add-btn");
const list = document.querySelector("#list-container");
const span = document.querySelector("span");

button.addEventListener("click", () => {
  if (input.value.length === 0) return console.log("Input is empty");
  list.insertAdjacentHTML(
    "beforeend",
    `<li>${input.value}<span>\u00d7</span></li>`
  );
  input.value = "";
  saveData();
});

list.addEventListener("click", (event) => {
  {
    event.target.tagName === "LI" &&
      (event.target.classList.toggle("checked"), saveData());
  }
  {
    event.target.tagName === "SPAN" &&
      (event.target.parentElement.remove(), saveData());
  }
});

const saveData = () => {
  localStorage.setItem("data", list.innerHTML);
};

const showTasks = () => {
  list.innerHTML = localStorage.getItem("data");
};

showTasks();
