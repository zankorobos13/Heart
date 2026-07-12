const WIDTH=64;
const HEIGHT=32;

const pixels=new Array(WIDTH*HEIGHT).fill(false);

const canvas=document.getElementById("canvas");
const ctx=canvas.getContext("2d");
ctx.imageSmoothingEnabled=false;

let drawing=false;
let color=true;

function redraw(){

    ctx.fillStyle="white";
    ctx.fillRect(0,0,WIDTH,HEIGHT);

    for(let i=0;i<pixels.length;i++){

        if(pixels[i]){
            const x=i%WIDTH;
            const y=Math.floor(i/WIDTH);

            ctx.fillStyle="black";
            ctx.fillRect(x,y,1,1);
        }

    }

}

redraw();

function draw(clientX,clientY){

    const rect=canvas.getBoundingClientRect();

    const x=Math.floor((clientX-rect.left)/rect.width*WIDTH);
    const y=Math.floor((clientY-rect.top)/rect.height*HEIGHT);

    if(x<0||x>=WIDTH||y<0||y>=HEIGHT)
        return;

    pixels[y*WIDTH+x]=color;

    ctx.fillStyle=color?"black":"white";
    ctx.fillRect(x,y,1,1);

}

canvas.onmousedown=e=>{
    drawing=true;
    draw(e.clientX,e.clientY);
};

window.onmouseup=()=>drawing=false;

canvas.onmousemove=e=>{
    if(drawing)
        draw(e.clientX,e.clientY);
};

canvas.addEventListener("touchstart", function (e) {
    drawing = true;
    draw(e.touches[0].clientX, e.touches[0].clientY);
}, { passive: true });

canvas.addEventListener("touchmove", function (e) {
    e.preventDefault(); // запрещаем только прокрутку во время рисования
    if (drawing) {
        draw(e.touches[0].clientX, e.touches[0].clientY);
    }
}, { passive: false });

canvas.addEventListener("touchend", function () {
    drawing = false;
});

canvas.addEventListener("touchcancel", function () {
    drawing = false;
});
blackBtn.onclick=()=>{
    color=true;
    blackBtn.classList.add("active");
    whiteBtn.classList.remove("active");
};

whiteBtn.onclick=()=>{
    color=false;
    whiteBtn.classList.add("active");
    blackBtn.classList.remove("active");
};

clearBtn.onclick=()=>{
    pixels.fill(false);
    redraw();
};

document.getElementById("imageForm").onsubmit = function () {

    const bytes = new Uint8Array(pixels.length / 8);

    for (let i = 0; i < pixels.length; i++) {
        if (pixels[i]) {
            bytes[i >> 3] |= (1 << (i & 7));
        }
    }

    // Кодируем в Base64
    let binary = "";
    for (const b of bytes) {
        binary += String.fromCharCode(b);
    }

    document.getElementById("message").value = btoa(binary);
};