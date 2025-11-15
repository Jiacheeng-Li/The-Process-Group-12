from flask import Flask
from flask import render_template
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy.orm import Mapped, mapped_column, DeclarativeBase
from datetime import date

app=Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///income_expense.db'
db = SQLAlchemy(app)

class User(db.Model):
    __tablename__ = "user"
    id: Mapped[int] = mapped_column(db.Integer, primary_key=True, autoincrement=True)
    username: Mapped[str] = mapped_column(db.String(50), nullable=False)
    password: Mapped[str] = mapped_column(db.String(200))

with app.app_context():
    db.create_all()

@app.route("/")
def index():
    return render_template("index.html")

if __name__=='__main__':
    app.run(debug = True)