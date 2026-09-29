async function simulateInput(button, state) {
  const response = await fetch("http://localhost:8000", {
    method: "POST",
    body: JSON.stringify({ button, state }),
  });
}

function getButtonEvent(key) {
  switch (key) {
    case "ArrowUp":
      return { el: document.getElementById("up-button"), button: "UP" };
    case "ArrowDown":
      return { el: document.getElementById("down-button"), button: "DOWN" };
    case "j":
      return { el: document.getElementById("a-button"), button: "A" };
    case "k":
      return { el: document.getElementById("b-button"), button: "B" };
    default:
      return null;
  }
}

document.addEventListener("keydown", async (event) => {
  const buttonEvent = getButtonEvent(event.key);

  if (!buttonEvent) return;

  const { el, button } = buttonEvent;

  await simulateInput(button, "PRESSED");
  el?.classList.add("input__button--pressed");
});

document.addEventListener("keyup", async (event) => {
  const buttonEvent = getButtonEvent(event.key);

  if (!buttonEvent) return;

  const { el, button } = buttonEvent;

  await simulateInput(button, "RELEASED");
  el?.classList.remove("input__button--pressed");
});
